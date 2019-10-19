#include "data_storage_manager.h"

namespace ustc_dbms {

  DSMgr::DSMgr() {
    std::cout << "DSMgr: " << "Start Data Storage Manager ..." << std::endl;
  }

  DSMgr::~DSMgr() {
    if (this->db_ptr_.size() > 0) {
      this->ReleaseDbPtr();
    }
    std::cout << "DSMgr: " << "Shutdown Data Storage Manager ..." << std::endl;
  }

  bool DSMgr::OpenDbFile(std::string file_name) {
    std::fstream *db_file = new std::fstream;
    db_file->open(file_name,std::ios::in|std::ios::out);
    if (!db_file->is_open()) {
      std::cerr << "DSMgr: " << __func__ << " read/write db file failed !" << std::endl;
      return false;
    }

    assert(this->db_ptr_.size() == 0);
    this->db_ptr_.push_back(db_file);
    return true;
  }

  bool DSMgr::CloseDbFile() {
    assert(this->db_ptr_.size() == 1);
    this->db_ptr_[0]->close();
    this->ReleaseDbPtr();
    this->db_ptr_.clear();
    return true;
  }

  DbFrame DSMgr::ReadPage(int page_id) const {
    int page_offset = page_id * DB_PAGE_SIZE;
    this->Seek(page_offset, DB_SEEK_BEG, false);

    DbPage db_page;
    this->db_ptr_[0]->read(&(db_page.page_), DB_PAGE_SIZE);
    if (*(this->db_ptr_[0])) {
      std::cout << "DSMgr: <" << __func__ << "> read page(" << page_id << ") successfully !" << std::endl;
    } else {
      std::cerr << "DSMgr: <" << __func__ << "> error, only read " << this->db_ptr_[0]->gcount() << " bytes !" << std::endl;
    }

    DbFrame db_frame;
    db_frame.frame_.push_back(std::make_pair(page_id, db_page));
    return db_frame;
  }

  bool DSMgr::WritePage(int frame_id, DbFrame frame) {
    if (frame.dirty_) {
      for (auto page : frame) {
        int page_id = page.first;
        DbPage db_page = page.second;
        int page_offset = page_id * DB_PAGE_SIZE;
        this->Seek(page_offset, DB_SEEK_BEG, true);
        this->db_ptr_[0]->write(&(db_page.page_), DB_PAGE_SIZE);
        std::cout << "DSMgr: <" << __func__ << "> write page(" << page_id << ") successfully !" << std::endl;
      }
    } else {
      std::cout << "DSMgr: <" << __func__ << "> page(" << page_id << ") dirty is false !" << std::endl;
    }
    return true;
  }

  void DSMgr::Seek(int offset, DB_SEEK_E pos, int is_put) {
    auto db_ptr = this->db_ptr_[0];
    if (is_put) {
      /* \brief Seek write file. */
      switch(pos) {
        case DB_SEEK_BEG:
          db_ptr->seekp(offset, std::ios::beg);
          break;
        case DB_SEEK_CUR:
          db_ptr->seekp(offset, std::ios::cur);
          break;
        case DB_SEEK_END:
          db_ptr->seekp(offset, std::ios::end);
          break;
        default:
          std::cerr << "DSMgr: " << __func__ << " seek type error !" << std::endl;
          break;
      }
    } else {
      /* \brief Seek read file. */
      switch(pos) {
        case DB_SEEK_CUR:
          db_ptr->seekg(offset, std::ios::cur);
          break;
        case DB_SEEK_BEG:
          db_ptr->seekg(offset, std::ios::beg);
          break;
        case DB_SEEK_END:
          db_ptr->seekg(offset, std::ios::end);
          break;
        default:
          std::cerr << "DSMgr: " << __func__ << " seek type error !" << std::endl;
          break;
      }      
    }
  }

  std::fstream * DSMgr::GetFile() {
    if (this->db_ptr_.size() > 0) {
      return this->db_ptr_[0];
    } else {
      return NULL;
    }
  }

}