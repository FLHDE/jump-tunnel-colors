#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define DllImport __declspec(dllimport)

DllImport UINT CreateID(LPCSTR str);

namespace Universe
{
    struct ISystem
    {};

    DllImport ISystem const * get_system(UINT id);
}

class DllImport INI_Reader
{
public:
    INI_Reader();
    ~INI_Reader();

    bool open(LPCSTR path, bool throwExceptionOnFail = false);
    bool read_header();
    bool is_header(LPCSTR name);
    bool read_value();
    bool is_value(LPCSTR name);
    bool get_value_bool(UINT index);
    int get_value_int(UINT index);
    LPCSTR get_value_string(UINT index);
    void close();

private:
    BYTE data[0x1565];
};

