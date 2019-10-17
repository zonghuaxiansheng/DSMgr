#ifndef _USTC_DATA_STORAGE_MANAGER_H_
#define _USTC_DATA_STORAGE_MANAGER_H_

#include "types.h"
#include <iostream>
#include <fstream>
#include <string>

namespace ustc_dbms {

enum DB_SSEK_E {DB_SEEK_BEG, DB_SEEK_CUR, DB_SEEK_END};

class DSMgr {
  public:
    DSMgr(bool split=false);
    bool OpenDbFile(std::string filename);
    bool CloseDbFile(int file_index=0);
    DbFrame ReadPage(int page_id);
    bool WritePage(int frame_id, DbFrame frame);
    void Seek(int offset, int pos, int is_put);
    std::fstream * GetFile(int file_index=0);
    void IncNumPages();
    int GetNumPages();
    void SetUse(int index, int use_bit); 
    int GetUse(int index);
    void ReleaseDbPtr() {
      for (auto db_ptr : this->db_ptr_) {
        delete db_ptr;
      }
    }
  private:
    /*
     * \brief db_split for multiple files
     * \now only support single file.
     */
    bool db_split_;
    /*
     * \brief db_ptr is a handle of db files.
     */
    std::vector<std::fstream *> db_ptr_;
    std::vector<int> pages_;
    // FILE *currFile;
    // int pages_num_;
    // int pages[MAXPAGES];    
};    // class DSMgr

}   // namespace ustc_dbms

#endif