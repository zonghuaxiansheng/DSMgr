#ifndef _USTC_BUFFER_MANAGER_H_
#define _USTC_BUFFER_MANAGER_H_

#include <map>
#include "types.h"
#include "data_storage_manager.h"

namespace ustc_dbms {

enum FRAME_STATUS_E {EMPTY, CLEAN, DIRTY};
enum CLOCK_STATUS_E {FIRST, LAST};

#ifdef USE_MULT_PAGES
typedef FRAME_STATUS_E PAGE_STATUS_E;
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
 * \brief Frame control block, with single page.
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
  /* \brief The frame clock status. */
  CLOCK_STATUS_E clock_status_;
  /* \brief Page data ptr. */
  char* dptr_;
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
  /* \brief The clock pointer of the buffer. */
  std::vector<FCB>::iterator clk_iter_;

  // BCB
  BCB(int bsize=1024) {
    this->buffer_size_ = bsize;
  }
  // SetBufferSize
  inline void SetBufferSize(int size) {
    this->buffer_size_ = size;
  }
  // InitBcb
  inline void InitBcb(int bsize=0) {
    if (bsize > 0) {
      this->SetBufferSize(bsize);
    } else {
      __HPRINT__({
      std::cout << "BMgr: " << __FUNC__ 
                << " Forget set buffer_size_ ? buffer_size_ will be old value("
                << this->buffer_size_ << ") !" << std::endl;
      })
    }
    // Initial BCB
    this->bcb_.clear();
    for (int i = 0; i < this->buffer_size_; i ++) {
      FCB fcb;
      fcb.frame_id_ = i;
      fcb.count_ = 0;
      fcb.frame_status_ = FRAME_STATUS_E::EMPTY;
      fcb.clock_status_ = CLOCK_STATUS_E::LAST;
      fcb.dptr_ = new char[DB_PAGE_SIZE * sizeof(char)];
      this->bcb_.push_back(fcb);
    }
    this->clk_iter_ = this->bcb_.begin();
  }
  // ReleaseBcb
  inline void ReleaseBcb() {
    for (auto& bcb_iter : this->bcb_) {
      delete bcb_iter.dptr_;
      // bcb_iter.dptr_ = nullptr;
    }
  }
  inline auto& GetBcb() {
    return this->bcb_;
  }
  // IncrClk
  inline void IncrClk() {
    if (this->clk_iter_ == this->bcb_.end()) {
      this->clk_iter_ = this->bcb_.begin();
    } else {
      this->clk_iter_ ++;
    }
  }
  // PickFcbOut
  inline FCB& PickFcbOut() {
    /*
     * \brief Turn the clock and search which has LAST tag, then pick it out. 
     * \Step1. Search from current state to end state.
     * \Step2. Search from start state to current state.
     */
    __MPRINT__({
    std::cout << "BMgr: " << __FUNC__
              << " Clock scheduling start !"
              << std::endl;
    })
    int loop_cnt = 0;
    while (true) {
      for (auto iter = this->clk_iter_; iter != this->bcb_.end(); iter ++) {
        if (iter->clock_status_ == CLOCK_STATUS_E::LAST) {
          __MPRINT__({
          std::cout << "<Last>[" << iter->frame_id_ << "] " << std::endl;
          })
          this->clk_iter_ = iter;
          this->IncrClk();
          return *iter;
        } else if (iter->count_ == 0) {
          iter->clock_status_ = CLOCK_STATUS_E::LAST;
          __MPRINT__({
          std::cout << "<First>[" << iter->frame_id_ << "] ";
          })
        } else {
          /* \brief This frame's count > 0 */
          __MPRINT__({
          std::cout << "<Skip>[" << iter->frame_id_ << "] ";
          })
        }
      }
      for (auto riter = this->bcb_.begin(); riter != this->clk_iter_; riter ++) {
        if (riter->clock_status_ == CLOCK_STATUS_E::LAST) {
          __MPRINT__({
          std::cout << "<Last>[" << riter->frame_id_ << "] " << std::endl;
          })
          this->clk_iter_ = riter;
          this->IncrClk();
          return *riter;
        } else if (riter->count_ == 0) {
          riter->clock_status_ = CLOCK_STATUS_E::LAST;
          __MPRINT__({
          std::cout << "<First>[" << riter->frame_id_ << "] ";
          })
        } else {
          /* \brief This frame's count > 0 */
          __MPRINT__({
          std::cout << "<Skip>[" << riter->frame_id_ << "] ";
          })
        }
      }
      loop_cnt ++;
      if (loop_cnt >= 2) {
        break;
      }
    }
    std::cerr << "BMgr: " << __FUNC__
              << " Frames are used by users, can't find out one frame to repalce !"
              << std::endl;
    exit(1);
    return *this->clk_iter_;
  }
  // GetFcb
  inline FCB& GetFcb(int index) {
    assert(index < this->buffer_size_);
    auto& fcb = this->bcb_[index];
    return fcb;
  }
  // CopyFcb
  inline void CopyFcb(FCB& src_fcb, FCB& dst_fcb) {
    dst_fcb.frame_id_ = src_fcb.frame_id_;
    dst_fcb.page_id_ = src_fcb.page_id_;
    dst_fcb.count_ = src_fcb.count_;
    dst_fcb.frame_status_ = src_fcb.frame_status_;
    dst_fcb.clock_status_ = src_fcb.clock_status_;
    dbCopy(src_fcb.dptr_, 0, dst_fcb.dptr_, 0, DB_PAGE_SIZE);
  }
  // UpdateFcb
  inline bool UpdateFcb(int index, FCB& fcb) {
    assert(index < this->buffer_size_);
    auto& up_fcb = this->bcb_[index];
    this->CopyFcb(fcb, up_fcb);
    return true;
  }
  // ResetFcb
  inline bool ResetFcb(int index) {
    assert(index < this->buffer_size_);
    auto& fcb = this->bcb_[index];
    fcb.frame_status_ = FRAME_STATUS_E::EMPTY;
    fcb.clock_status_ = CLOCK_STATUS_E::LAST;
    return true;
  }
  // GetFreeNum
  inline int GetFreeNum() {
    int free_cnt = 0;
    for (auto fcb : this->bcb_) {
      if (fcb.frame_status_ == FRAME_STATUS_E::EMPTY) {
        free_cnt ++;
      }
    }
    return free_cnt;
  }
};    // struct BCB

/*
 * \brief Hash bucket.
 */
struct HashBucket {
  /* \brief Buckets of each <page_id, frame_id> item. */
  std::vector<std::map<int, int>> bucket_;
  /* \brief Bucket size. */
  int bucket_size_;
  /* \brief Use an array to point the frame in which bucket. */
  int* frame_ptr_;

  // HashBucket
  HashBucket(int bsize=128) {
    this->bucket_size_= bsize;
  }
  // InitBucket
  inline void InitBucket(int bsize=0,
                         int fsize=1024) {
    if (bsize > 0) {
      this->bucket_size_ = bsize;
    } else {
      __HPRINT__({
      std::cout << "BMgr: " << __FUNC__
                << " Forget set bucket_size_ ? bucket_size_ will be old value("
                << this->bucket_size_ << ") !" << std::endl;
      })
    }
    this->bucket_.clear();
    for (int i = 0; i < this->bucket_size_; i ++) {
      std::map<int, int> map_;
      map_.clear();
      this->bucket_.push_back(map_);
    }
    // New frame pointer
    this->frame_ptr_ = new int[fsize * sizeof(int)];
    memset(this->frame_ptr_, -1, fsize * sizeof(int));
  }
  // ReleaseBucket
  inline void ReleaseBucket() {
    delete this->frame_ptr_;
  }
  // Hash
  inline int Hash(int page_id) {
    return page_id % this->bucket_size_;
  }
  // isExist
  inline auto isExist(int page_id) -> std::pair<int, bool> {
    auto index = this->Hash(page_id);
    auto& map_ = this->bucket_[index];
    auto iter = map_.find(page_id);
    if (iter != map_.end()) {
      return std::make_pair(iter->second, true);
    } else {
      return std::make_pair(0, false);
    }
  }
  // Insert
  inline bool Insert(std::pair<int, int> in_pair) {
    auto [page_id, frame_id] = in_pair;
    auto index = this->Hash(page_id);
    auto& map_ = this->bucket_[index];
    map_.insert(in_pair);
    this->frame_ptr_[frame_id] = index;
    return true;
  }
  // Delete
  inline bool Delete(int frame_id) {
    auto index = this->frame_ptr_[frame_id];
    if (index >= 0) {
      auto& map_ = this->bucket_[index];
      for (auto iter = map_.begin(); iter != map_.end(); iter ++) {
        if (iter->second == frame_id) {
          map_.erase(iter);
          return true;
        }
      } 
    }
    __MPRINT__({
    std::cout << "BMgr: " << __FUNC__
              << " Delete bucket item failed with (frame_id,index)[" 
              << frame_id << "," << index << "] !"
              << std::endl;
    })
    return false;
  }
  // Print
  inline bool Print() {
    std::cout << "BMgr: " << __FUNC__
              << " * * * * HashBucket * * * *"
              << std::endl;
    for (auto map_ : this->bucket_) {
      std::cout << "Sub_Bucket: <page_id , frame_id> ";
      for (auto item : map_) {
        std::cout << "<" << item.first << ", " << item.second << "> ";
      }
      std::cout << std::endl;
    }
  }
};

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
     * \brief A structure of hash bucket.
     */
    HashBucket* hash_bucket_;
    /*
     * \brief Data storage manager.
     */
    DSMgr* db_dsmgr_;
    /* 
     * \brief The size of the buffer.
     */
    int buffer_size_;
  public:
    BMgr(int bsize=1024,
         int hsize=128,
         std::string db_path="out/default.db",
         bool is_build=true);
    ~BMgr();

    bool InitBMgrTest(int test_size);

    // Interface functions
    int FixPage(int page_id, int prot);
    auto FixNewPage() -> std::pair<int, int>;
    int UnfixPage(int page_id);
    int WritePage(int page_id);
    int NumFreeFrames();
    void PrintIO();
    // Internal Functions
    // int SelectVictim();
    // void RemoveBCB(BCB * ptr, int page_id);
    // void RemoveLRUEle(int frid);
    void SetDirty(int frame_id);
    void SetClean(int frame_id);
    void WriteDirtys();
    void PrintFrame(int frame_id);
};    // class BMgr

}   // namespace ustc_dbms

#endif
