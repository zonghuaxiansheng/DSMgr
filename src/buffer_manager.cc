#include "buffer_manager.h"

namespace ustc_dbms {
BMgr::BMgr(int bsize,
           int fsize,
           std::string db_path) {
  std::cout << "BMgr: " << "Start Buffer Manager ..." << std::endl;
  // Initial buffer size and frame size
  this->buffer_size_ = bsize;
  this->frame_size_ = fsize;
  // Initial BCB
  this->db_bcb_ = new BCB(bsize, fsize);
  this->db_bcb_->InitBcb(bsize);
  // Allocate memory space for buffer
  this->db_buffer_ = new char[(bsize * fsize) * (DB_PAGE_SIZE * sizeof(char))];
  // Initial DSMgr
  // std::string db_path = "./out/default.db";
  this->db_dsmgr_ = new DSMgr(db_path);
}
BMgr::~BMgr() {
  delete this->db_bcb_;
  delete this->db_buffer_;
  delete this->db_dsmgr_;
  std::cout << "BMgr: " << "Shutdown Buffer Manager ..." << std::endl;
}
bool BMgr::InitBMgrTest(int test_size) {
  for (int i = 0; i < test_size; i ++) {
    // New one page
    auto page_id = this->db_dsmgr_->NewPage();
    // Store page info into BCB
    auto index = this->Hash(page_id);
    auto& fcb = this->db_bcb_->GetFcb(index);
    fcb.page_id_ = page_id;
    fcb.frame_status_ = FRAME_STATUS_E::CLEAN;
    // Store page data into buffer
    auto db_frame = this->db_dsmgr_->ReadPage(page_id, 1);
    auto page_data = db_frame.frame_[0].second;
    for (int i = 0; i < DB_PAGE_SIZE; i ++) {
      this->db_buffer_[fcb.frame_id_*DB_PAGE_SIZE + i] = page_data.page_[i];
    }
  }
}
int BMgr::Hash(int page_id) {
  return page_id % this->buffer_size_;
}
int BMgr::FixPage(int page_id, int port) {

  std::cout << "BMgr: " << __FUNC__ 
            << " page_id[" << page_id << "]" << std::endl;

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
  std::cout << "BMgr: " << __FUNC__ 
            << " FixPage need call DSMgr to read page back !" << std::endl;
  // Read page from DSMgr
  auto db_frame = this->db_dsmgr_->ReadPage(page_id, 1);
  // Replace FCB info
  // TODO: Change prelace algorithm.
  auto index = this->Hash(page_id);
  auto& fcb = this->db_bcb_->GetFcb(index);
  if (fcb.frame_status_ == FRAME_STATUS_E::DIRTY) {
    /*
     * \brief Frame is dirty now, need write back.
     */
  }
  fcb.page_id_ = page_id;
  fcb.frame_status_ = FRAME_STATUS_E::CLEAN;
  // Store page data into buffer
  auto page_data = db_frame.frame_[0].second;
  for (int i = 0; i < DB_PAGE_SIZE; i ++) {
    this->db_buffer_[fcb.frame_id_*DB_PAGE_SIZE + i] = page_data.page_[i];
  }
#endif
  // this->WriteIntoPage(page_data, pindex);
  return index;
}
}
