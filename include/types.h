#ifndef _USTC_TYPES_H_
#define _USTC_TYPES_H_

#include <assert.h>
#include <utility>
#include <vector>
#include <iostream>

#define FRAMESIZE 4096
#define DEFBUFSIZE  1024

#define DB_FRAME_NUM 4
#define DB_PAGE_SIZE 4096

#define __FUNC__ "_" << __func__ << "_L" << __LINE__ << "_"

namespace ustc_dbms {

struct DbPage {
  /*!
   *\ brief one page contains DBLOCK_SIZR bytes.
   */
  char page_[DB_PAGE_SIZE];
#if 0
  /*! \ brief one record size. */
  int record_size_;
  /*! \ brief the slot status of used. */
  std::vector<int> slot_index_;

  int GetSlotSize() const {
    return slot_index_.size();
  }
  int GetRecordSize() const {
    return record_size_;
  }
  void SetRecordSize(int record_size) const {
    record_size_ = record_size;
  }
#endif
};    // struct DbPage

struct DbFrame {
  /*!
   *\ brief one frame contains FRAME_LINE_NUM block.
   */
  std::vector<std::pair<int, DbPage>> frame_;
  bool dirty_;
};    // struct DbFrame

/*
 * \brief Use for normally print.
 */
template <typename ... Args>
void dbCout(Args&&... args) {
  (std::cout << ... << args) << std::endl;
}

/*
 * \brief Use for buffer data copy.
 */
void dbCopy(char* src_data,
            int src_offset,
            char* dst_data,
            int dst_offset,
            int copy_size) {
  for (int i = 0; i < copy_size; i ++) {
    dst_data[dst_offset + i] = src_data[src_offset + i];
  }
}

}   // namespace ustc_dbms


#endif
