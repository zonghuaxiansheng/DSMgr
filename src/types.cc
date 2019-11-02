#include "types.h"

namespace ustc_dbms {
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
