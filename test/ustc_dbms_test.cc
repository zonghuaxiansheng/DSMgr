#include "buffer_manager.h"
// #include "data_storage_manager.h"
#include <iostream>

int main(void) {

  int buffer_size = 128;
  int bucket_size = 16;
  std::string db_path("out/ustc.db");

  std::cout << "***********************USTC DBMS TEST***********************" << std::endl;
  ustc_dbms::BMgr bmgr(buffer_size,
                       bucket_size,
                       db_path);
  bmgr.InitBMgrTest(1024);
  bmgr.FixPage(6, 0);
  bmgr.FixPage(20, 0);
  auto [page_id, frame_id] = bmgr.FixNewPage();
  std::cout << "Test: " << __FUNC__
            << " FixNewPage return with page_id[" << page_id
            << "] frame_id[" << frame_id << "]"
            << std::endl;
  // ...
  std::cout << "***********************USTC DBMS TEST***********************" << std::endl;
  
  return 0;
}
