#include "buffer_manager.h"

namespace ustc_dbms {
BMgr::BMgr(int bsize=1024, int fsize=1) {
  std::cout << "BMgr: " << "Start Buffer Manager ..." << std::endl;
  this->buffer_size_ = bsize;
  this->frame_size_ = fsize;
  this->db_bcb_ = new BCB(bsize, fsize);
  this->db_buffer_ = new char[(bsize * fsize) * (DB_PAGE_SIZE * sizeof(char))];
  this->db_dsmgr_ = new DSMgr();
}
BMgr::~BMgr() {
  delete this->db_bcb_;
  delete this->db_buffer_;
  delete this->db_dsmgr_;
  std::cout << "BMgr: " << "Shutdown Buffer Manager ..." << std::endl;
}
int BMgr::Hash(int page_id) {
  return page_id % this->buffer_size_;
}
int BMgr::FixPage(int page_id, int port) {
  auto index = this->Hash(page_id);
  auto& fcb = this->db_bcb_->GetFcb(index);

#ifdef USE_MULT_PAGES
  for (auto fpcb : fcb.frame_pcb_) {
    if (fpcb.page_id_ == page_id) {
      return fpcb.frame_id_;
    }
  }
#else
  if (fcb.page_id_ == page_id) {
    return fcb.frame_id_;
  }
#endif

#ifdef USE_MULT_PAGES
  int pindex = 0;
  auto page_data = this->db_dsmgr_->ReadPage(page_id, 1);
  auto frame_size = fcb.frame_pcb_.size();
  if (frame_size < this->frame_size_) {
    /*
     * \brief The frame has valid space to put new page. 
     */
    FPCB fpcb;
    fpcb.page_id_ = page_id;
    fpcb.frame_id_ = index;
    fpcb.page_status_ = PAGE_STATUS_E::CLEAN;
    fcb.frame_pcb_.push_back(fpcb);
    pindex = frame_size;
  } else {
    /*
     * \brief The frame is full now, need to find one page to replace.
     */
    bool is_replace = true;
    // TODO: Change replace algorithm.
    for (int i = 0; i < this->frame_size_; i ++){ 
    // for (auto& fpcb : fcb.frame_pcb_) {
      auto& fpcb = fcb.frame_pcb_[i];
      if (fpcb.page_status_ != PAGE_STATUS_E::DIRTY) {
        fpcb.page_id_ = page_id;
        fpcb.frame_id_ = index;
        fpcb.page_status_ = PAGE_STATUS_E::CLEAN;
        pindex = i;
        is_replace = false;
      }
    }
    if (is_replace) {
      // TODO: Change replace algorithm.
      pindex = 0;
      auto& fpcb = fcb.frame_pcb_[pindex];
      this->WriteBackPage(fpcb, pindex);
      fpcb.page_id_ = page_id;
      fpcb.frame_id_ = index;
      fpcb.page_status_ = PAGE_STATUS_E::CLEAN;
    } 
  }
#else
#endif
  thie->WriteIntoPage(page_data, pindex);
  return index;
}
}