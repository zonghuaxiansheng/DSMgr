#ifndef _USTC_DATA_STORAGE_MANAGER_H_
#define _USTC_DATA_STORAGE_MANAGER_H_

#include "string.h"
#include "math.h"
#include "types.h"
#include <iostream>
#include <fstream>
#include <string>
#include <map>

namespace ustc_dbms {

typedef unsigned char uint8_t;

enum DB_SEEK_E {DB_SEEK_BEG, DB_SEEK_CUR, DB_SEEK_END};

struct PCB {
  /* \brief A bit map for all pages.Valid is 1, invalid is 0. */
  std::vector<uint8_t> bit_map_;
  /* \brief Virtual page to physical page convert. */
  std::map<int, int> vp_cvt_;
  /* \brief Page num. */
  int page_num_;

  // GetPageNum
  int GetPageNum() const {
    return this->page_num_;
  }
  // V2PConvert
  std::pair<int, bool> V2PConvert(int v_id) const {
    auto p_id_iter = this->vp_cvt_.find(v_id);
    if (p_id_iter == this->vp_cvt_.end()) {
      return std::make_pair(NULL, false);
    } else {
      auto p_id = p_id_iter->second;
      return std::make_pair(p_id, true);
    }
  }
  // GetMaxVPage
  int GetMaxVPage() {
    int max_v_id = 0;
#if 0
    for (auto vp_item : this->vp_cvt_) {
      if (vp_item.first > max_v_id) {
        max_v_id = vp_item.first;
      }
    }
#else
    if (this->vp_cvt_.size() > 0) {
      auto end_iter = this->vp_cvt_.end();
      end_iter --;
      max_v_id = end_iter->first;
    } else {
      max_v_id = 0;
    }
#endif
    return max_v_id;
  }
  // GetValidPageIndex
  std::pair<int, bool> GetValidPageIndex() {
    for (int i; i < this->bit_map_.size(); i++) {
      if (this->bit_map_[i]) {
        return std::make_pair(i, true);
      }
    }
    return std::make_pair(0, false);
  }
  // NewOnePage
  int NewOnePage() {
    this->bit_map_.push_back((uint8_t)1);
    auto new_p_id = this->bit_map_.size() - 1;
    return new_p_id;
  }
  // SetBitMap
  int SetBitMap(const int index, const uint8_t value) {
    assert(this->bit_map_.size() > index);
    this->bit_map_[index] = value;
  }
  // DelOnePage
  bool DelOnePage(const int v_id) {
    // Convert page_id from v_id to p_id.
    auto [p_id, p_valid] = V2PConvert(v_id);
    assert(p_valid);
    assert(this->bit_map_.size() > p_id);
    // Set page as valid.
    this->bit_map_[p_id] = (uint8_t)1;
    // Delete the (v_id -> p_id)'s relationship.
    auto eret = this->vp_cvt_.erase(v_id);
    assert(eret);
    // Sub page num.
    this->page_num_ --;
    return true;
  }
  // IncrOnePage
  int IncrOnePage() {
    // Get one valid page, if p_valid is false,
    // then new one page.
    auto [p_id, p_valid] = GetValidPageIndex();
    if (!p_valid) {
      p_id = NewOnePage();
    }
    // Set bit_map_[p_id] = used
    SetBitMap(p_id, 0);
    // Get a new v_id
    auto v_id = GetMaxVPage();
    if (v_id > 0 || p_id > 0) {
      v_id ++;
    }
    __MPRINT__({
    std::cout << "DSMgr: " << __FUNC__ 
              << " logical_id[" << v_id 
              << "] -> pyhsical_id[" << p_id << "]" << std::endl;
    })
    // Insert [v_id,p_id] into v2p map.
    this->vp_cvt_.insert(std::make_pair(v_id, p_id));
    // Increase page num.
    this->page_num_ ++;
    return v_id;
  }
};    // Page Control Block

class DSMgr {
  private:
    /*
     * \brief A structure of page control block.
     */
    PCB db_pcb_;
    /*
     * \brief A structure of db_file's handle.
     */
    std::vector<std::fstream *> db_ptr_;
    /* \brief Read cnt. */
    int r_cnt_;
    /* \brief Write cnt. */
    int w_cnt_;
  public:
    DSMgr(std::string db_path="./out/default.db",
          bool is_build=true);
    ~DSMgr();
    /*
     * \brief Functoins of db file operation.
     */
    bool OpenDbFile(std::string file_name);
    bool CloseDbFile(int file_index=0);
    DbFrame ReadPage(int page_id, int page_num);
    bool WritePage(int frame_id, DbFrame& frame);
    void Seek(int offset, DB_SEEK_E pos, int is_put);
    std::fstream * GetFile();
    /*
     * \brief Functions of PCB operation.
     */
    int NewPage() {
      auto page_id = this->db_pcb_.IncrOnePage();
      DbPage db_page;
      memset(db_page.page_, '0', DB_PAGE_SIZE);
      DbFrame db_frame;
      db_frame.frame_.push_back(std::make_pair(page_id, db_page));
      db_frame.dirty_ = true;
      this->WritePage(0, db_frame);
      return page_id;
    }
    bool DelPage(int page_id) {
      auto dret = this->db_pcb_.DelOnePage(page_id);
      // assert(dret);
      return dret;
    }
    int GetPageNum() {
      auto page_num = this->db_pcb_.GetPageNum();
      return page_num;
    }
    void ResetIO() {
      this->r_cnt_ = 0;
      this->w_cnt_ = 0;
    }
    void IncrICnt() {
      this->r_cnt_ ++;
    }
    void IncrOCnt() {
      this->w_cnt_ ++;
    }
    void PrintIO() {
      std::cout << "DSMgr: " << __FUNC__
                << std::endl
                << "* IO Details: <I>[" << this->r_cnt_ << "] <O>["
                << this->w_cnt_ << "] <Total>[" << (this->r_cnt_ + this->w_cnt_) << "]"
                << std::endl;
    }
    /*
     * \brief Release the db_handle's memory space.
     */
    void ReleaseDbPtr() {
      for (auto& db_ptr : this->db_ptr_) {
        delete db_ptr;
      }
    }
};    // class DSMgr
}   // namespace ustc_dbms

#endif
