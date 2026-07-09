// scan.h

#ifndef SCAN_H
#define SCANTYPE_ACTIVE_H

int compareBSSID(const void *a, const void *b);

class Scan {
public:
  
  int scan_start(void);
  uint8_t getScanCount(void);
  simple_scan_result_t *getFilteredScanResults(void);
  static int compareBSSID(const void *a, const void *b);
  int removeDuplicates(void);

protected:

private:
  simple_scan_result_t filtered_scan_results[MAX_SCAN_ENTRIES] = {};

};
// EOF
#endif

