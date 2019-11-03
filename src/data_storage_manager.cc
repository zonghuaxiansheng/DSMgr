#include "data_storage_manager.h"

namespace ustc_dbms {

DSMgr::DSMgr(std::string db_path,
             bool is_build) {
  std::cout << "DSMgr: " << "Start Data Storage Manager ..." << std::endl;
  this->ResetIO();
  this->OpenDbFile(db_path);
  if (!is_build) {
    this->Bytes2Pcb();
  }
}
DSMgr::~DSMgr() {
  if (this->db_ptr_.size() > 0) {
    // Dump meta data.
    this->Pcb2Bytes();
    this->CloseDbFile();
    // this->ReleaseDbPtr();
  }
  std::cout << "DSMgr: " << "Shutdown Data Storage Manager ..." << std::endl;
}
bool DSMgr::OpenDbFile(std::string file_name) {
  std::fstream *db_file = new std::fstream;
  db_file->open(file_name, std::ios::in|std::ios::out);
  if (!db_file->is_open()) {
    std::cerr << "DSMgr: " << __FUNC__
              << " Read/Write db file failed !"
              << std::endl;
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
    this->IncrICnt();
    if (*(this->db_ptr_[0])) {
      __MPRINT__({
      std::cout << "DSMgr: " << __FUNC__
                << " logical_id[" << incr_id << "] -> physical_id[" << p_id << "]"
                << std::endl;
      })
    } else {
      std::cerr << "DSMgr: " << __FUNC__
                << " error, only read " << this->db_ptr_[0]->gcount() << " bytes !"
                << std::endl;
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
      this->IncrOCnt();
      __MPRINT__({
      std::cout << "DSMgr: " << __FUNC__
                << " logical_id[" << v_id << "] -> physical_id[" << p_id << "]"
                << std::endl;
      })
    }
  } else {
    std::cout << "DSMgr: " << __FUNC__
              << " frame[" << frame_id << "] dirty is false !"
              << std::endl;
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
void DSMgr::Pcb2Bytes() {
  int page_num = this->db_pcb_.bit_map_.size();
  int bit_map_num = ceil(page_num*sizeof(uint8_t) / (DB_PAGE_SIZE + 0.0));
  int vp_cvt_num = ceil(page_num*sizeof(int)*2 / (DB_PAGE_SIZE + 0.0));

  int meta_block_num = bit_map_num + vp_cvt_num;
  int data_block_num = page_num;

  uint8_t* bit_meta_block = new uint8_t[bit_map_num*DB_PAGE_SIZE*sizeof(uint8_t)];
  uint8_t* vp_meta_block = new uint8_t[vp_cvt_num*DB_PAGE_SIZE*sizeof(uint8_t)];
  uint8_t* head_meta_block = new uint8_t[DB_PAGE_SIZE*sizeof(uint8_t)];
  for (int i = 0; i < page_num; i ++) {
    bit_meta_block[i] = this->db_pcb_.bit_map_[i];
  }
  int offset = 0;
  for (auto& vp : this->db_pcb_.vp_cvt_) {
    int v_id = vp.first;
    int p_id = vp.second;
    Int2Char(v_id, vp_meta_block + offset);
    offset += 4;
    Int2Char(p_id, vp_meta_block + offset);
    offset += 4;
  }
  offset = 0;
  Int2Char(meta_block_num, head_meta_block + offset);
  offset += 4;
  Int2Char(data_block_num, head_meta_block + offset);
  offset += 4;
  Int2Char(page_num, head_meta_block + offset);

  this->Seek(0, DB_SEEK_E::DB_SEEK_END, true);
  this->db_ptr_[0]->write(bit_meta_block, bit_map_num*DB_PAGE_SIZE);
  this->db_ptr_[0]->write(vp_meta_block, vp_cvt_num*DB_PAGE_SIZE);
  this->db_ptr_[0]->write(head_meta_block, DB_PAGE_SIZE);

  delete bit_meta_block;
  delete vp_meta_block;
  delete head_meta_block;
}
void DSMgr::Bytes2Pcb() {
  this->Seek(-DB_PAGE_SIZE, DB_SEEK_E::DB_SEEK_END, false);
  uint8_t* head_meta_block = new uint8_t[DB_PAGE_SIZE*sizeof(uint8_t)];
  this->db_ptr_[0].read(head_meta_block, DB_PAGE_SIZE);

  int offset = 0;
  int meta_block_num;
  int data_block_num;
  int page_num;
  Char2Int(head_meta_block + offset, meta_block_num);
  offset += 4;
  Char2Int(head_meta_block + offset, data_block_num);
  offset += 4;
  Char2Int(head_meta_block + offset, page_num);

  int bit_map_num = ceil(page_num*sizeof(uint8_t) / (DB_PAGE_SIZE + 0.0));
  int vp_cvt_num = ceil(page_num*sizeof(int)*2 / (DB_PAGE_SIZE + 0.0));

  uint8_t* bit_meta_block = new uint8_t[bit_map_num*DB_PAGE_SIZE*sizeof(uint8_t)];
  uint8_t* vp_meta_block = new uint8_t[vp_cvt_num*DB_PAGE_SIZE*sizeof(uint8_t)];
  uint8_t* head_meta_block = new uint8_t[DB_PAGE_SIZE*sizeof(uint8_t)];

  this->Seek(-(bit_map_num + vp_cvt_num + 1)*DB_PAGE_SIZE, DB_SEEK_E::DB_SEEK_END, false);
  this->db_ptr_[0]->read(bit_meta_block, bit_map_num*DB_PAGE_SIZE);
  this->db_ptr_[0]->read(vp_meta_block, vp_cvt_num*DB_PAGE_SIZE);
  
  this->db_pcb_.bit_map_.clear();
  for (int i = 0; i < page_num; i ++) {
    this->db_pcb_.bit_map_.push_back(bit_meta_block[i]);
  }
  this->db_pcb_.vp_cvt_.clear();
  for (int i = 0; i < page_num; i ++) {
    int v_id;
    int p_id;
    Char2Int(vp_meta_block + i*sizeof(int)*2, v_id);
    Char2Int(vp_meta_block + i*sizeof(int)*2 + 4, p_id);
    this->db_pcb_.insert(std::make_pair(v_id, p_id));
  }
  
  delete bit_meta_block;
  delete vp_meta_block;
  delete head_meta_block;
}

}
