#include "World.hpp"
#include "Script.hpp"
#include "Class.hpp"
#include "Procedure.hpp"
#include "Resource.hpp"
#include "Passes/StackReconstructionPass.hpp"
#include "Passes/SplitSendPass.hpp"
#include "Passes/AnalyzeMessagePass.hpp"
#include "Passes/AnalyzeObjectsPass.hpp"
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Intrinsics.h>

#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>

using namespace llvm;


BEGIN_NAMESPACE_SCI


static World s_world;


static void DumpScriptModule(uint id)
{
    std::error_code ec;
    Module *m = GetWorld().getScript(id)->getModule();
    m->print(raw_fd_ostream(("C:\\Temp\\SCI\\" + m->getName()).str() + ".ll", ec, sys::fs::F_Text), nullptr);
}


World& GetWorld()
{
    return s_world;
}


World::World() :
    m_dataLayout(""),
    m_ctx(getGlobalContext()),
    m_classes(nullptr),
    m_classCount(0)
{
    m_sizeTy = Type::getInt32Ty(m_ctx);

    Type *i16Ty = Type::getInt16Ty(m_ctx);
    Type *elems[] = {
        m_sizeTy,                   // vftbl
        i16Ty,                      // species
        i16Ty,                      // super
        i16Ty,                      // info
        m_sizeTy,                   // name
        ArrayType::get(m_sizeTy, 0) // vars
    };
    m_absClassTy = StructType::create(m_ctx, elems);
}


World::~World()
{
    if (m_classes != nullptr)
    {
        for (uint i = 0, n = m_classCount; i < n; ++i)
        {
            if (m_classes[i].m_type != nullptr)
            {
                m_classes[i].~Class();
            }
        }
        free(m_classes);
    }
}


ConstantInt* World::getConstantValue(int16_t val) const
{
    ConstantInt *c;
    if (IsUnsignedValue(val))
    {
        c = ConstantInt::get(getSizeType(), static_cast<uint64_t>(static_cast<uint16_t>(val)), false);
    }
    else
    {
        c = ConstantInt::get(getSizeType(), static_cast<uint64_t>(static_cast<int16_t>(val)), true);
    }
    return c;
}


void World::setDataLayout(const llvm::DataLayout &dl)
{
    m_dataLayout = dl;
}


bool World::load()
{
    ResClassEntry *res = (ResClassEntry *)ResLoad(RES_VOCAB, CLASSTBL_VOCAB);
    if (res == nullptr)
    {
        return false;
    }

    m_classCount = ResHandleSize(res) / sizeof(ResClassEntry);
    size_t size = m_classCount * sizeof(Class);
    m_classes = reinterpret_cast<Class *>(malloc(size));
    memset(m_classes, 0, size);
    for (uint i = 0, n = m_classCount; i < n; ++i)
    {
        Script **classScript = reinterpret_cast<Script **>(&m_classes[i].m_super + 1);
        *classScript = acquireScript(res[i].scriptNum);
    }

    ResUnLoad(RES_VOCAB, CLASSTBL_VOCAB);

    for (uint i = 0; i < 1000; ++i)
    {
        Script *script = acquireScript(i);
        if (script != nullptr)
        {
            script->load();
        }
    }



  //  DumpScriptModule(999);
    StackReconstructionPass().run();
    SplitSendPass().run();
  //  AnalyzeObjectsPass().run();
    AnalyzeMessagePass().run();

    DumpScriptModule(999);
    DumpScriptModule(997);
    DumpScriptModule(255);
    DumpScriptModule(0);
    return true;
}


Script* World::acquireScript(uint id)
{
    Script *script = m_scripts[id].get();
    if (script == nullptr)
    {
        Handle hunk = ResLoad(RES_SCRIPT, id);
        if (hunk != nullptr)
        {
            script = new Script(id, hunk);
            m_scripts[id].reset(script);
        }
    }
    return script;
}


Script* World::getScript(uint id) const
{
    return m_scripts[id].get();
}


Script* World::getScript(Module &module) const
{
    Script *script = nullptr;
    StringRef name = module.getName();
    if (name.size() == 9 && name.startswith("Script"))
    {
        if (isdigit(name[8]) && isdigit(name[7]) && isdigit(name[6]))
        {
            uint id = (name[6] - '0') * 100 +
                      (name[7] - '0') * 10 +
                      (name[8] - '0') * 1;

            script = m_scripts[id].get();
            assert(script == nullptr || script->getModule() == &module);
        }
    }
    return script;
}


Object* World::lookupObject(GlobalVariable &var) const
{
    Script *script = getScript(*var.getParent());
    return (script != nullptr) ? script->lookupObject(var) : nullptr;
}


StringRef World::getSelectorName(uint id)
{
    return m_sels.getSelectorName(id);
}


Class* World::getClass(uint id)
{
    if (id >= m_classCount)
    {
        return nullptr;
    }

    Class *cls = &m_classes[id];
    if (cls->m_type == nullptr)
    {
        if (!cls->m_script.load() || cls->m_type == nullptr)
        {
            cls = nullptr;
        }
    }
    return cls;
}


Class* World::addClass(const ObjRes &res, Script &script)
{
    assert(selector_cast<uint>(res.speciesSel) < m_classCount);
    assert(&script == &m_classes[res.speciesSel].m_script);

    Class *cls = &m_classes[res.speciesSel];
    if (cls->m_type == nullptr)
    {
        new(cls) Class(res, script);
    }
    return cls;
}


bool World::registerProcedure(Procedure &proc)
{
    bool ret = false;
    Function *func = proc.getFunction();
    if (func != nullptr)
    {
        Procedure *&slot = m_funcMap[func];
        ret = (slot == nullptr);
        slot = &proc;
    }
    return ret;
}


Procedure* World::getProcedure(const Function &func) const
{
    return m_funcMap.lookup(&func);
}


uint World::getGlobalVariablesCount() const
{
    return m_scripts[0]->getLocalVariablesCount();
}


END_NAMESPACE_SCI