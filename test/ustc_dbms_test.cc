#include "buffer_manager.h"
// #include "data_storage_manager.h"
#include <iostream>

bool runBMgrTraceTest(ustc_dbms::BMgr& bmgr,
                      std::string& trace_path) {
  std::fstream trace_;
  trace_.open(trace_path, std::ios::in);
  if (!trace_.is_open()) {
    std::cerr << "Test: " << __FUNC__
              << " Read trace file failed !"
              << std::endl;
    return false;
  }
}

int main(void) {

  int buffer_size = 128;
  int bucket_size = 16;
  std::string db_path("out/ustc.db");

  std::cout << "***********************USTC DBMS TEST***********************" << std::endl;
  ustc_dbms::BMgr bmgr(buffer_size,
                       bucket_size,
                       db_path);
  bmgr.InitBMgrTest(1024);
  for (int i = 1; i < 100; i ++) {
    bmgr.FixPage(i, 0);
  }
  bmgr.SetDirty(10);
  bmgr.SetDirty(20);
  bmgr.PrintFrame(10);
  bmgr.SetClean(10);
  bmgr.PrintFrame(10);
  bmgr.WriteDirtys();
  auto [page_id, frame_id] = bmgr.FixNewPage();
  std::cout << "Test: " << __FUNC__
            << " FixNewPage return with page_id[" << page_id
            << "] frame_id[" << frame_id << "]"
            << std::endl;
  std::cout << "***********************USTC DBMS TEST***********************" << std::endl;
  
  return 0;
}
