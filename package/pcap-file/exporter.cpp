#include <nan.h>
#include <fstream>
#include <../../plugkit/include/plugkit/file.h>

using namespace plugkit;

FileStatus exportFile(Context *ctx,
                                    const char *filename,
                                    const RawFrame *frames,
                                    size_t length,
                                    FileExporterCallback callback)
{
  std::ofstream ofs(filename, std::ios::binary);
  if (!ofs.is_open()) {
   return FILE_STATUS_ERROR;
  }

  while (1) {
    size_t length;
    const RawFrame *begin = callback(&length);
    if (length == 0) break;
  }

  return FILE_STATUS_DONE;
}

void Init(v8::Local<v8::Object> exports) {
  static FileExporter exporter;
  exporter.func = exportFile;
  exports->Set(Nan::New("exporter").ToLocalChecked(),
               Nan::New<v8::External>(&exporter));
}

NODE_MODULE(pcapExporter, Init);
