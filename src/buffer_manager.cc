#include "buffer_manager.h"

namespace ustc_dbms {
BMgr::BMgr(int bsize,
           int hsize,
           std::string db_path) {
  std::cout << "BMgr: " << "Start Buffer Manager ..." << std::endl;
  // Initial buffer size
  this->buffer_size_ = bsize;
  // Initial BCB
  this->db_bcb_ = new BCB(bsize);
  this->db_bcb_->InitBcb(bsize);
  // Initial HashBucket
  this->hash_bucket_ = new HashBucket(hsize);
  this->hash_bucket_->InitBucket(hsize, bsize);
  // Initial DSMgr
  // std::string db_path = "./out/default.db";
  this->db_dsmgr_ = new DSMgr(db_path);
}
BMgr::~BMgr() {
  // Release the memory space which buffer has allocated.
  this->db_bcb_->ReleaseBcb();
  // Release the frame pointer array which hash_bucket has allocated.
  this->hash_bucket_->ReleaseBucket();
  // Delete structure of BCB and DSMgr.
  delete this->db_bcb_;
  delete this->hash_bucket_;
  delete this->db_dsmgr_;
  std::cout << "BMgr: " << "Shutdown Buffer Manager ..." << std::endl;
}
bool BMgr::InitBMgrTest(int test_size) {
  for (int i = 0; i < test_size; i ++) {
    // New one page, then load page data.
    auto page_id = this->db_dsmgr_->NewPage();
    auto rd_frame = this->db_dsmgr_->ReadPage(page_id, 1);
    auto& page_data = rd_frame.frame_[0].second;
    // Get one frame from BCB, then put info & data into frame.
    auto& fcb = this->db_bcb_->PickFcbOut();
    fcb.page_id_ = page_id;
    fcb.count_ = 0;
    fcb.frame_status_ = FRAME_STATUS_E::CLEAN;
    fcb.clock_status_ = CLOCK_STATUS_E::FIRST;
    dbCopy(page_data.page_, 0, fcb.dptr_, 0, DB_PAGE_SIZE);
    // Put <page_id, frame_id> into HashBucket.
    this->hash_bucket_->Insert(std::make_pair(fcb.page_id_, fcb.frame_id_));
  }
}
int BMgr::FixPage(int page_id,
                  int prot) {
  std::cout << "BMgr: " << __FUNC__ 
            << " Fix page_id[" << page_id << "]" 
            << std::endl;
  auto [frame_id, exist] = this->hash_bucket_->isExist(page_id);
  // Buffer hit.
  if (exist) {
    // Update FCB count.
    auto& fcb = this->db_bcb_->GetFcb(frame_id);
    fcb.count_ ++;
    return frame_id;
  }
  // Buffer miss.
  std::cout << "BMgr: " << __FUNC__ 
            << " Buffer miss, need call DSMgr to read page back !" 
            << std::endl;
  // Read page from DSMgr
  auto rd_frame = this->db_dsmgr_->ReadPage(page_id, 1);
  auto& page_data = rd_frame.frame_[0].second;
  // Using Clock algorithm to replace FCB.
  auto& fcb = this->db_bcb_->PickFcbOut();
  if (fcb.frame_status_ == FRAME_STATUS_E::DIRTY) {
    /*
     * \brief Frame is dirty now, need write back.
     */
    DbPage db_page;
    dbCopy(fcb.dptr_, 0, db_page.page_, 0, DB_PAGE_SIZE);
    DbFrame db_frame;
    db_frame.frame_.push_back(std::make_pair(fcb.page_id_, db_page));
    db_frame.dirty_ = true;
    this->db_dsmgr_->WritePage(0, db_frame);
  }
  // Update FCB info & data.
  fcb.page_id_ = page_id;
  fcb.count_ = 1;
  fcb.frame_status_ = FRAME_STATUS_E::CLEAN;
  fcb.clock_status_ = CLOCK_STATUS_E::FIRST;
  dbCopy(page_data.page_, 0, fcb.dptr_, 0, DB_PAGE_SIZE);
  // Debug
  this->hash_bucket_->Print();
  // Delete the bucket item which contains value=frame_id
  this->hash_bucket_->Delete(fcb.frame_id_);
  // Insert a new item into bucket.
  this->hash_bucket_->Insert(std::make_pair(fcb.page_id_, fcb.frame_id_));
  return fcb.frame_id_;
}
auto BMgr::FixNewPage() -> std::pair<int, int> {
  // Call DSMgr to new one page, then return meta data of the page.
  auto page_id = this->db_dsmgr_->NewPage();
  std::cout << "BMgr: " << __FUNC__
            << " Call DSMgr and return logical_id[" << page_id << "]"
            << std::endl;
  auto frame_id = this->FixPage(page_id, 0);
  return std::make_pair(page_id, frame_id);
}
int BMgr::UnfixPage(int page_id) {
  std::cout << "BMgr: " << __FUNC__
            << " Unfix page_id[" << page_id << "]"
            << std::endl;
  auto [frame_id, exist] = this->hash_bucket_->isExist(page_id);
  if (exist) {
    // Buffer hit.
    // Update FCB count.
    auto& fcb = this->db_bcb_->GetFcb(frame_id);
    if (fcb.count_ > 0) {
      fcb.count_ --;
    }
    return frame_id;
  } else {
    // Buffer miss.
    // Do nothing.
    std::cout << "BMgr :" << __FUNC__
              << " Unfix with a page which is not exist at buffer, so do nothing !"
              << std::endl;
    return 0;
  }
}
int BMgr::NumFreeFrames() {
  return this->db_bcb_->GetFreeNum();
}
void BMgr::SetDirty(int frame_id) {
  auto& fcb = this->db_bcb_->GetFcb(frame_id);
  fcb.frame_status_ = FRAME_STATUS_E::DIRTY;
}
void BMgr::SetClean(int frame_id) {
  auto& fcb = this->db_bcb_->GetFcb(frame_id);
  if (fcb.frame_status_ == FRAME_STATUS_E::DIRTY) {
    std::cout << "BMgr: " << __FUNC__
              << " Set a dirty frame to clean is dangerous !"
              << std::endl;
  }
  fcb.frame_status_ = FRAME_STATUS_E::CLEAN;
}
void BMgr::WriteDirtys() {
  // Write back all dirty frames.
  for (auto& fcb : this->db_bcb_->bcb_) {
    if (fcb.frame_status_ == FRAME_STATUS_E::DIRTY) {
      DbPage db_page;
      dbCopy(fcb.dptr_, 0, db_page.page_, 0, DB_PAGE_SIZE);
      DbFrame db_frame;
      db_frame.frame_.push_back(std::make_pair(fcb.page_id_, db_page));
      db_frame.dirty_ = true;
      this->db_dsmgr_->WritePage(0, db_frame);
    }
  }
}
void BMgr::PrintFrame(int frame_id) {
  auto& fcb = this->db_bcb_->GetFcb(frame_id);
  std::cout << "BMgr: " << __FUNC__
            << " * * * * * * * Frame * * * * * *"
            << std::endl;
  std::cout << "* frame_id: " << fcb.frame_id_ << std::endl
            << "* page_id: " << fcb.page_id_ << std::endl
            << "* count: " << fcb.count_ << std::endl
            << "* frame_status: " << fcb.frame_status_ << std::endl
            << "* clock_status: " << fcb.clock_status_ << std::endl;
}
}
