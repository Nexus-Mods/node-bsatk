#include "bsatk/src/bsaarchive.h"
#include "string_cast.h"
#include <vector>
#include <napi.h>

const char *convertErrorCode(BSA::EErrorCode code) {
  switch (code) {
    case BSA::ERROR_ACCESSFAILED: return "access failed";
    case BSA::ERROR_CANCELED: return "canceled";
    case BSA::ERROR_FILENOTFOUND: return "file not found";
    case BSA::ERROR_INVALIDDATA: return "invalid data";
    case BSA::ERROR_INVALIDHASHES: return "invalid hashes";
    case BSA::ERROR_SOURCEFILEMISSING: return "source file missing";
    case BSA::ERROR_ZLIBINITFAILED: return "zlib init failed";
    case BSA::ERROR_NONE: return nullptr;
    default: return "unknown";
  }
}

class BSAException : public std::exception {
public:
  BSAException(BSA::EErrorCode code, const char *sysCall)
    : m_Code(code), m_Errno(errno), m_SysCall(sysCall) {}

  virtual char const* what() const override {
    return convertErrorCode(m_Code);
  }

  BSA::EErrorCode code() const { return m_Code; }
  int sysError() const { return m_Errno; }
  const char *sysCall() const { return m_SysCall; }

private:
  BSA::EErrorCode m_Code;
  int m_Errno;
  const char *m_SysCall;
};

class BSAddon : public Napi::Addon<BSAddon> {
public:
  BSAddon(Napi::Env env, Napi::Object exports);

  Napi::FunctionReference constructArchive;
  Napi::FunctionReference constructFolder;
  Napi::FunctionReference constructFile;

private:
  Napi::Value loadBSA(const Napi::CallbackInfo& info);
  Napi::Value createBSA(const Napi::CallbackInfo& info);
};

class ExtractWorker : public Napi::AsyncWorker {
public:
  ExtractWorker(std::shared_ptr<BSA::Archive> archive,
             BSA::File::Ptr file,
             const char *outputrDirectory,
             const Napi::Function &appCallback)
    : Napi::AsyncWorker(appCallback)
    , m_Archive(archive)
    , m_File(file)
    , m_OutputDirectory(outputrDirectory)
  {}

  void Execute() {
    BSA::EErrorCode code;
    if (m_File.get() != nullptr) {
      code = m_Archive->extract(m_File, m_OutputDirectory.c_str());
    }
    else {
      code = m_Archive->extractAll(m_OutputDirectory.c_str(),
        [](int, std::string) { return true; });
    }
    if (code != BSA::ERROR_NONE) {
      SetError(convertErrorCode(code));
    }
  }

  virtual void OnOK() override {
    Callback().Call(Receiver().Value(), std::initializer_list<napi_value>{ Env().Null() });
  }

private:
  std::shared_ptr<BSA::Archive> m_Archive;
  std::shared_ptr<BSA::File> m_File;
  std::string m_OutputDirectory;
};

class BSAFile : public Napi::ObjectWrap<BSAFile> {
public:
  static Napi::FunctionReference Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "BSAFile", {
      InstanceMethod("getName", &BSAFile::getName),
      InstanceMethod("getFilePath", &BSAFile::getFilePath),
      InstanceMethod("getFileSize", &BSAFile::getFileSize),
      });

    exports.Set("BSAFile", func);

    return Napi::Persistent(func);
  }

  BSAFile(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<BSAFile>(info)
  {
    Ref();
  }

  BSAFile(const Napi::CallbackInfo &info, std::shared_ptr<BSA::File> file)
    : Napi::ObjectWrap<BSAFile>(info)
    , m_File(file)
  {
    Ref();
  }

  ~BSAFile() {
    Unref();
  }

  static Napi::Object CreateNewItem(Napi::Env env) {
    BSAddon* addon = env.GetInstanceData<BSAddon>();
    return addon->constructFile.New({ });
  }

  void setWrappee(const std::shared_ptr<BSA::File>& file)
  {
    m_File = file;
  }

  BSA::File::Ptr getWrappee() const { return m_File; }

  Napi::Value getName(const Napi::CallbackInfo &info) { return Napi::String::New(info.Env(), m_File->getName()); }
  Napi::Value getFilePath(const Napi::CallbackInfo &info) { return Napi::String::New(info.Env(), m_File->getFilePath()); }
  Napi::Value getFileSize(const Napi::CallbackInfo &info) { return Napi::Number::New(info.Env(), m_File->getFileSize()); }

private:
  BSA::File::Ptr m_File;
};

class BSAFolder: public Napi::ObjectWrap<BSAFolder> {
public:
  static Napi::FunctionReference Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "BSAFolder", {
      InstanceMethod("getName", &BSAFolder::getName),
      InstanceMethod("getFullPath", &BSAFolder::getFullPath),
      InstanceMethod("getNumSubFolders", &BSAFolder::getNumSubFolders),
      InstanceMethod("getSubFolder", &BSAFolder::getSubFolder),
      InstanceMethod("getNumFiles", &BSAFolder::getNumFiles),
      InstanceMethod("countFiles", &BSAFolder::countFiles),
      InstanceMethod("getFile", &BSAFolder::getFile),
      InstanceMethod("addFile", &BSAFolder::addFile),
      InstanceMethod("addFolder", &BSAFolder::addFolder),
      });

    exports.Set("BSAFolder", func);

    return Napi::Persistent(func);
  }

  BSAFolder(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<BSAFolder>(info)
  {
    Ref();
  }

  BSAFolder(const Napi::CallbackInfo &info, std::shared_ptr<BSA::Folder> folder)
    : Napi::ObjectWrap<BSAFolder>(info)
    , m_Folder(folder)
  {
    Ref();
  }

  virtual ~BSAFolder() {
    Unref();
  }

  static Napi::Object CreateNewItem(Napi::Env env) {
    BSAddon* addon = env.GetInstanceData<BSAddon>();
    return addon->constructFolder.New({ });
  }

  void setWrappee(const std::shared_ptr<BSA::Folder>& folder)
  {
    m_Folder = folder;
  }

  Napi::Value getName(const Napi::CallbackInfo &info) { return Napi::String::New(info.Env(), m_Folder->getName()); }
  Napi::Value getFullPath(const Napi::CallbackInfo &info) { return Napi::String::New(info.Env(), m_Folder->getFullPath()); }
  Napi::Value getNumSubFolders(const Napi::CallbackInfo &info) { return Napi::Number::New(info.Env(), m_Folder->getNumSubFolders()); }
  Napi::Value getSubFolder(const Napi::CallbackInfo &info) {
    int32_t idx = info[0].ToNumber().Int32Value();
    Napi::Object result = CreateNewItem(info.Env());
    BSAFolder::Unwrap(result)->setWrappee(m_Folder->getSubFolder(idx));
    return result;
  }
  Napi::Value getNumFiles(const Napi::CallbackInfo &info) { return Napi::Number::New(info.Env(), m_Folder->getNumFiles()); }
  Napi::Value countFiles(const Napi::CallbackInfo &info) { return Napi::Number::New(info.Env(), m_Folder->countFiles()); }
  Napi::Value getFile(const Napi::CallbackInfo &info) {
    int32_t idx = info[0].ToNumber().Int32Value();
    Napi::Object result = BSAFile::CreateNewItem(info.Env());
    BSAFile::Unwrap(result)->setWrappee(m_Folder->getFile(idx));
    return result;
  }
  Napi::Value addFile(const Napi::CallbackInfo &info) {
    BSAFile *file = BSAFile::Unwrap(info[0].ToObject());
    m_Folder->addFile(file->getWrappee());
    return info.Env().Undefined();
  }
  Napi::Value addFolder(const Napi::CallbackInfo &info) {
    return BSAFolder(info, m_Folder->addFolder(Napi::String::New(info.Env(), info[0].ToString().Utf8Value()))).Value();
  }

private:
  std::shared_ptr<BSA::Folder> m_Folder;
};

class BSArchive: public Napi::ObjectWrap<BSArchive> {
public:
  static Napi::FunctionReference Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "BSArchive", {
      InstanceMethod("getType", &BSArchive::getType),
      InstanceMethod("getRoot", &BSArchive::getRoot),
      InstanceMethod("write", &BSArchive::write),
      InstanceMethod("extractFile", &BSArchive::extractFile),
      InstanceMethod("extractAll", &BSArchive::extractAll),
      });

    Napi::FunctionReference* constructor = new Napi::FunctionReference();
    exports.Set("BSArchive", func);
    return Napi::Persistent(func);
  }

  BSArchive(const BSArchive&) = delete;

  BSArchive(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<BSArchive>(info)
    , m_Wrapped(new BSA::Archive())
    , m_Name(info[0].ToString())
  {
    Ref();
  }

  static Napi::Object CreateNewItem(Napi::Env env) {
    BSAddon* addon = env.GetInstanceData<BSAddon>();
    return addon->constructArchive.New({ });
  }

  ~BSArchive() {
    Unref();
  }

  void readAsync(const Napi::CallbackInfo& info, const std::string& filePath, bool testHashes, const Napi::Function& cb) {
    const Napi::Env env = info.Env();
    std::thread* loadThread;

    m_ThreadCB = Napi::ThreadSafeFunction::New(info.Env(), cb,
      "AsyncLoadCB", 0, 1, [loadThread](Napi::Env) {
      loadThread->join();
      delete loadThread;
    });


    loadThread = new std::thread{ [this, env, filePath, testHashes]() {
      auto callback = [](Napi::Env env, Napi::Function jsCallback, BSArchive* result) {
        jsCallback.Call({ env.Null(), result->Value() });
      };

      auto callbackError = [](Napi::Env env, Napi::Function jsCallback, std::string* err = nullptr) {
        jsCallback.Call({Napi::String::New(env, err->c_str())});
      };

      try {
        read(filePath.c_str(), testHashes);
        m_ThreadCB.Acquire();
        m_ThreadCB.BlockingCall(this, callback);
      }
      catch (const std::exception& e) {
        std::string* err = new std::string{ e.what() };
        m_ThreadCB.Acquire();
        m_ThreadCB.BlockingCall(err, callbackError);
      }

      m_ThreadCB.Release();
    } };
  }

  Napi::Value createFile(const Napi::CallbackInfo &info, const char *name, const char *sourcePath, bool compressed) {
    return BSAFile(info, m_Wrapped->createFile(name, sourcePath, compressed)).Value();
  }

  Napi::Value write(const Napi::CallbackInfo &info) {
    BSA::EErrorCode err = m_Wrapped->write(m_Name.c_str());
    if (err != BSA::ERROR_NONE) {
      throw std::runtime_error(convertErrorCode(err));
    }
    return info.Env().Undefined();
  }

  Napi::Value getRoot(const Napi::CallbackInfo &info) {
    BSA::Folder::Ptr root = m_Wrapped->getRoot();
    Napi::Object result = BSAFolder::CreateNewItem(info.Env());
    BSAFolder::Unwrap(result)->setWrappee(root);

    return result;
  }

  Napi::Value getType(const Napi::CallbackInfo& info) {
    switch (m_Wrapped->getType()) {
      case BSA::TYPE_OBLIVION: return Napi::String::From(info.Env(), "oblivion");
      case BSA::TYPE_SKYRIM:   return Napi::String::From(info.Env(), "skyrim");
        // fallout 3 and fallout nv use the same type as skyrim
      default: return info.Env().Null();
    }
  }

  Napi::Value extractFile(const Napi::CallbackInfo &info) {
    auto worker = new ExtractWorker(m_Wrapped,
      BSAFile::Unwrap(info[0].ToObject())->getWrappee(),
      info[1].ToString().Utf8Value().c_str(),
      info[2].As<Napi::Function>());

    worker->Queue();
    return info.Env().Undefined();
  }

  Napi::Value extractAll(const Napi::CallbackInfo &info) {
    std::string outputDirectory = info[0].ToString();
    Napi::Function callback = info[1].As<Napi::Function>();

    auto worker = new ExtractWorker(m_Wrapped,
      std::shared_ptr<BSA::File>(),
      outputDirectory.c_str(),
      callback);

    worker->Queue();
    return info.Env().Undefined();
  }

private:
  void read(const char *fileName, bool testHashes) {
    BSA::EErrorCode err = m_Wrapped->read(toWC(fileName, CodePage::UTF8, strlen(fileName)).c_str(), testHashes);
    if (err != BSA::ERROR_NONE) {
      throw std::runtime_error(convertErrorCode(err));
    }
  }
private:
  std::string m_Name;
  std::shared_ptr<BSA::Archive> m_Wrapped;
  Napi::ThreadSafeFunction m_ThreadCB;
};

BSAddon::BSAddon(Napi::Env env, Napi::Object exports) {
  DefineAddon(exports, {
    InstanceMethod("loadBSA", &BSAddon::loadBSA),
    InstanceMethod("createBSA", &BSAddon::createBSA),
    });
  constructArchive = BSArchive::Init(env, exports);
  constructFolder = BSAFolder::Init(env, exports);
  constructFile = BSAFile::Init(env, exports);
}

Napi::Value BSAddon::loadBSA(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  Napi::String filePath = info[0].ToString();
  Napi::Boolean testHashes = info[1].ToBoolean();
  Napi::Function cb = info[2].As<Napi::Function>();

  Napi::Object result = BSArchive::CreateNewItem(env);
  BSArchive* resultObj = BSArchive::Unwrap(result);

  resultObj->readAsync(info, filePath, testHashes, cb);

  return info.Env().Undefined();
}

Napi::Value BSAddon::createBSA(const Napi::CallbackInfo& info) {
  Napi::String filePath = info[0].ToString();
  Napi::Function cb = info[1].As<Napi::Function>();

  Napi::Object result = BSArchive::CreateNewItem(info.Env());
  cb.Call({ result });

  return info.Env().Undefined();
}

NODE_API_ADDON(BSAddon)
