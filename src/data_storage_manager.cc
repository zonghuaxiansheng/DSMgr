#include "data_storage_manager.h"

namespace ustc_dbms {

  DSMgr::DSMgr(std::string db_path) {
    std::cout << "DSMgr: " << "Start Data Storage Manager ..." << std::endl;
    this->OpenDbFile(db_path);
  }

  DSMgr::~DSMgr() {
    if (this->db_ptr_.size() > 0) {
      this->CloseDbFile();
      // this->ReleaseDbPtr();
    }
    std::cout << "DSMgr: " << "Shutdown Data Storage Manager ..." << std::endl;
  }

  bool DSMgr::OpenDbFile(std::string file_name) {
    std::fstream *db_file = new std::fstream;
    db_file->open(file_name,std::ios::in|std::ios::out);
    if (!db_file->is_open()) {
      std::cerr << "DSMgr: " << __FUNC__ << " read/write db file failed !" << std::endl;
      return false;
    }

    assert(this->db_ptr_.size() == 0);
    this->db_ptr_.push_back(db_file);
    return true;
  }

  bool DSMgr::CloseDbFile(int file_index) {
    assert(this->db_ptr_.size() == 1);
    this->db_ptr_[0]->close();
    this->ReleaseDbPtr();
    this->db_ptr_.clear();
    return true;
  }

  DbFrame DSMgr::ReadPage(int page_id, int page_num) {
    int page_offset;
    DbFrame db_frame;
    for (int i = 0; i < page_num; i ++) {
      // Get page offset.
      int incr_id = page_id + i; 
      auto [p_id, p_valid] = this->db_pcb_.V2PConvert(incr_id);
      assert(p_valid == true);
      page_offset = p_id * DB_PAGE_SIZE;
      this->Seek(page_offset, DB_SEEK_BEG, false);
      // Read one page.
      DbPage db_page;
      this->db_ptr_[0]->read(db_page.page_, DB_PAGE_SIZE);
      if (*(this->db_ptr_[0])) {
        std::cout << "DSMgr: " << __FUNC__ << " logical_id[" << incr_id 
                  << "] -> physical_id[" << p_id << "]" << std::endl;
      } else {
        std::cerr << "DSMgr: " << __FUNC__ << " error, only read " \
                  << this->db_ptr_[0]->gcount() << " bytes !" << std::endl;
      }
      // Push page into frame.
      db_frame.frame_.push_back(std::make_pair(incr_id, db_page));
    }
    return db_frame;
  }

  bool DSMgr::WritePage(int frame_id, DbFrame& frame) {
    if (frame.dirty_) {
      for (auto page : frame.frame_) {
        int v_id = page.first;
        auto [p_id, p_valid] = this->db_pcb_.V2PConvert(v_id);
        assert(p_valid == true);
        int page_offset = p_id * DB_PAGE_SIZE;
        this->Seek(page_offset, DB_SEEK_BEG, true);

        DbPage db_page = page.second;
        this->db_ptr_[0]->write(db_page.page_, DB_PAGE_SIZE);
        std::cout << "DSMgr: " << __FUNC__ << " logical_id[" << v_id 
                  << "] -> physical_id[" << p_id << "]" << std::endl;
      }
    } else {
      std::cout << "DSMgr: " << __FUNC__ << " frame[" << frame_id << "] dirty is false !" << std::endl;
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
          std::cerr << "DSMgr: " << __FUNC__ << " seek type error !" << std::endl;
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
          std::cerr << "DSMgr: " << __FUNC__ << " seek type error !" << std::endl;
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
