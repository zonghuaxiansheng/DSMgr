#ifndef _USTC_DATA_STORAGE_MANAGER_H_
#define _USTC_DATA_STORAGE_MANAGER_H_

#include "types.h"
#include <iostream>
#include <string>

namespace ustc_dbms {

class DSMgr {
  public:
    DSMgr();
    int OpenFile(string filename);
    int CloseFile();
    bFrame ReadPage(int page_id);
    int WritePage(int frame_id, bFrame frm);
    int Seek(int offset, int pos);
    FILE * GetFile();
    void IncNumPages();
    int GetNumPages();
    void SetUse(int index, int use_bit); 
    int GetUse(int index);
  private:
    FILE *currFile;
    int numPages;
    int pages[MAXPAGES];    
};    // class DSMgr

}   // namespace ustc_dbms

#endif