#ifndef _USTC_BUFFER_MANAGER_H_
#define _USTC_BUFFER_MANAGER_H_

#include "types.h"

namespace ustc_dbms {

struct BCB {
  BCB();
  int page_id;
  int frame_id;
  int count;
  int dirty;
  BCB* next;
};    // struct BCB

class BMgr {
  public:
    BMgr();
    // Interface functions
    int FixPage(int page_id, int prot);
    void NewPage();
    void FixNewPage();
    int UnfixPage(int page_id);
    int NumFreeFrames();
    // Internal Functions
    int SelectVictim();
    int Hash(int page_id);
    void RemoveBCB(BCB * ptr, int page_id);
    void RemoveLRUEle(int frid);
    void SetDirty(int frame_id);
    void UnsetDirty(int frame_id);
    void WriteDirtys();
    void PrintFrame(int frame_id);
  private:
    // Hash Table
    int ftop[DEFBUFSIZE];
    BCB* ptof[DEFBUFSIZE];     
};    // class BMgr

}   // namespace ustc_dbms

#endif