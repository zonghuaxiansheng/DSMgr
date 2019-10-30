#ifndef _USTC_BUFFER_MANAGER_H_
#define _USTC_BUFFER_MANAGER_H_

#include "types.h"
#include "data_storage_manager.h"

namespace ustc_dbms {

enum FRAME_STATUS_E {EMPTY, CLEAN, DIRTY};
typedef FRAME_STATUS_E PAGE_STATUS_E;


#ifdef USE_MULT_PAGES
/*
 * \brief Frame's page control block.
 */
struct FPCB {
  /* \brief Page id. */
  int page_id_;
  /* \brief The frame id of the page. */
  int frame_id_;
  /* \brief Count. */
  int count_;
  /* \brief Page status. */
  PAGE_STATUS_E page_status_;
};    // struct FPCB

/*
 * \brief Frame control block,
 * \with multiply pages.
 */
struct FCB {
  /* \brief Vector of FPCB of one frame. */
  std::vector<FPCB> frame_pcb_;
  /* \brief Frame status. */
  FRAME_STATUS_E frame_status_;
  /* \brief The size of pages of one frame contian. */
  int frame_size_;
};    // struct FCB
#else
/*
 * \brief Frame control block,
 * \with single page.
 */
struct FCB {
  /* \brief Page id. */
  int page_id_;
  /* \brief The frame id of the page. */
  int frame_id_;
  /* \brief Count. */
  int count_;
  /* \brief Page status. */
  FRAME_STATUS_E frame_status_;
};
#endif

/*
 * \brief Buffer control block.
 */
struct BCB {
  /* \brief Vector of FCB. */
  std::vector<FCB> bcb_;
  /* \brief The size of frames of the buffer. */
  int buffer_size_;
  /* \brief The size of pages of one frame. */
  int frame_size_;
  /*
   * \brief BCB functions.
   */
  // BCB
  BCB(int bsize=1024, int fsize=1) {
    this->buffer_size_ = bsize;
    this->frame_size_ = fsize;    
  }
  // SetBufferSize
  inline void SetBufferSize(int size) {
    this->buffer_size_ = size;
  }
  // InitBcb
  inline void InitBcb(int buffer_size=0) {
    if (buffer_size > 0) {
      this->SetBufferSize(buffer_size);
    } else {
      std::cout << "BMgr: " << __func__ << " Forget set buffer_size_ ? buffer_size_ will be old value(" \
                << this->buffer_size_ << ") !" << std::endl;
    }
    this->bcb_.clear();
    for (int i = 0; i < this->buffer_size_; i ++) {
      FCB fcb;
      fcb.frame_id_ = i;
      fcb.count_ = 0;
      fcb.frame_status_ = FRAME_STATUS_E::EMPTY;
      this->bcb_.push_back(fcb);
    }
  }
  // GetFcb
  inline FCB& GetFcb(int index) {
    assert(index < this->buffer_size_);
    auto& fcb = this->bcb_[index];
    return fcb;
  }
  // UpdateFcb
  inline bool UpdateFcb(int index, FCB fcb) {
    assert(index < this->buffer_size_);
    this->bcb_[index] = fcb;
    return true;
  }
  // ResetFcb
  inline bool ResetFcb(int index) {
    assert(index < this->buffer_size_);
    auto& fcb = this->bcb_[index];
    fcb.frame_status_ = FRAME_STATUS_E::EMPTY;
    return true;
  }
};    // struct BCB

/*
 * \brief Buffer Manager.
 */
class BMgr {
  private:
    /*
     * \brief A structure of buffer control block.
     */
    BCB* db_bcb_;
    /*
     * \brief Buffer array point.
     */
    char* db_buffer_;
    /*
     * \brief Data storage manager.
     */
    DSMgr* db_dsmgr_;
    /* \brief The size of the buffer. */
    int buffer_size_;
    /* \brief The size of pages of one frame. */
    int frame_size_;
  public:
    BMgr(int bsize=1024,
         int fsize=1,
         std::string db_path="out/default.db");
    ~BMgr();
    int Hash(int page_id);
    bool InitBMgrTest(int test_size);

    // Interface functions
    int FixPage(int page_id, int prot);
    void NewPage();
    void FixNewPage();
    int UnfixPage(int page_id);
    int NumFreeFrames();
    // Internal Functions
    int SelectVictim();
    void RemoveBCB(BCB * ptr, int page_id);
    void RemoveLRUEle(int frid);
    void SetDirty(int frame_id);
    void UnsetDirty(int frame_id);
    void WriteDirtys();
    void PrintFrame(int frame_id);
};    // class BMgr

}   // namespace ustc_dbms

#endif
