#ifndef _USTC_TYPES_H_
#define _USTC_TYPES_H_

#include <utinity>

#define FRAMESIZE 4096
#define DEFBUFSIZE  1024

#define DB_FRAME_NUM 4
#define DB_PAGE_SIZE 4096

namespace ustc_dbms {

struct DbFrame {
  /*!
   *\ brief one frame contains FRAME_LINE_NUM block.
   */
  std::vector<std::pair<int, DbPage>> frame_;
  bool dirty_;
};    // struct DbFrame

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

}   // namespace ustc_dbms



#endif