#include "Stations.h"

Stations::Stations() {
  list = new LinkedList<Station>;
}

void Stations::add(uint8_t* mac, int accesspointNum) {
  int stationNum = findStation(mac);

  if (stationNum < 0) {
    internal_add(mac, accesspointNum);
    //print(list->size() - 1, list->size() == 1, false);
  } else {
    *getPkts(stationNum) += 1;
    *getTime(stationNum) = currentTime;
  }
  changed = true;
}

int Stations::findStation(uint8_t* mac) {
  uint8_t* station_i_mac;

  for (int i = 0; i < list->size(); i++) {
    if (memcmp(getMac(i), mac, 6) == 0)
      return i;
  }
  return -1;
}

void Stations::sort() {
  list->sort([](Station & a, Station & b) -> int{
    if (*a.pkts == *b.pkts) return 0;
    if (*a.pkts > *b.pkts) return -1;
    if (*a.pkts < *b.pkts) return 1;
  });
}

void Stations::removeAll() {
  internal_removeAll();
  prntln(ST_CLEARED_LIST);
  changed = true;
}

void Stations::remove(int num) {
  if (!check(num)) return;
  internal_remove(num);
  prnt(ST_REMOVED_STATION);
  prntln(num);
  changed = true;
}

void Stations::removeOldest() {
  int oldest = 0;
  for (int i = 1; i < list->size(); i++) {
    if (*getTime(i) > *getTime(oldest))
      oldest = i;
  }
  internal_remove(oldest);
  changed = true;
}

void Stations::printAll() {
  prntln(ST_HEADER);
  if (list->size() == 0)
    prntln(ST_LIST_EMPTY);
  else
    for (int i = 0; i < list->size(); i++)
      print(i, i == 0, i == list->size() - 1);
}

void Stations::printSelected() {
  prntln(ST_HEADER);
  int max = selected();

  if (max == 0) {
    prntln(ST_NO_DEVICES_SELECTED);
    return;
  }

  for (int i = 0, j = 0; i < list->size() && j < max; i++) {
    if (getSelected(i)) {
      print(i, j == 0, j == max - 1);
      j++;
    }
  }
}

void Stations::print(int num) {
  print(num, true, true);
}

void Stations::print(int num, bool header, bool footer) {
  if (!check(num)) return;
  if (header) {
    prntln(ST_TABLE_HEADER);
    prntln(ST_TABLE_DIVIDER);
  }

  prnt(buildString(String(),(String)num, 2));
  prnt(buildString(String(SPACE) + getMacStr(num), String(), 18));
  prnt(buildString(String(SPACE), (String)getCh(num), 3));
  prnt(buildString(String(SPACE) + getNameStr(num), String(), 17));
  prnt(buildString(String(SPACE) + getVendorStr(num), String(), 9));
  prnt(buildString(String(SPACE), (String)*getPkts(num), 9));
  prnt(buildString(String(SPACE) + getAPStr(num), String(), 33));
  prnt(buildString(String(SPACE) + getTimeStr(num), String(), 10));
  prntln(buildString(String(SPACE) + getSelectedStr(num), String(), 9));
  
  if (footer)
    prntln(ST_TABLE_DIVIDER);
}

String Stations::getAPStr(int num) {
  if (!check(num)) return String();
  return accesspoints.getSSID(getAP(num));
}

uint8_t Stations::getAP(int num) {
  if (!check(num)) return 0;
  return list->get(num).ap;
}

String Stations::getNameStr(int num) {
  if (!check(num)) return String();
  return names.find(getMac(num));
}

bool Stations::hasName(int num) {
  if (!check(num)) return false;
  return names.findID(getMac(num)) >= 0;
}

uint8_t* Stations::getMac(int num) {
  if (!check(num)) return 0;
  return list->get(num).mac;
}

String Stations::getMacStr(int num) {
  String value = "";
  if (check(num)) {
    uint8_t* mac = getMac(num);
    for (int i = 0; i < 6; i++) {
      if (mac[i] < 0x10) value += "0";
      value += String(mac[i], HEX);
      if (i < 5) value += ":";
    }
  }
  return value;
}

String Stations::getMacVendorStr(int num) {
  String value;
  if (check(num)){
    value = getVendorStr(num) + ":";
    uint8_t* mac = getMac(num);
    for (int i = 3; i < 6; i++) {
      if (mac[i] < 0x10) value += "0";
      value += String(mac[i], HEX);
      if (i < 5) value += ":";
    }
  }
  return value;
}

String Stations::getVendorStr(int num) {
  if (!check(num)) return String();
  return searchVendor(list->get(num).mac);
}

String Stations::getSelectedStr(int num) {
  return b2a(getSelected(num));
}

uint32_t* Stations::getPkts(int num) {
  if (!check(num)) return NULL;
  return list->get(num).pkts;
}

uint32_t* Stations::getTime(int num) {
  if (!check(num)) return NULL;
  return list->get(num).time;
}

String Stations::getTimeStr(int num) {
  if (!check(num)) return String();
  uint32_t difference = currentTime - *getTime(num);

  if (difference < 1000)
    return str(ST_SMALLER_ONESEC);
  else if (difference < 60000)
    return str(ST_SMALLER_ONEMIN);
  else {
    uint32_t minutes = difference / 60000;
    if (minutes > 60)
      return str(ST_BIGER_ONEHOUR);
    else
      return (String)minutes + str(STR_MIN);
  }

}

bool Stations::getSelected(int num) {
  if (!check(num)) return false;
  return list->get(num).selected;
}

uint8_t Stations::getCh(int num) {
  if (!check(num)) return 0;
  return list->get(num).ch;
}

void Stations::select(int num) {
  if (!check(num)) return;
  internal_select(num);
  prnt(ST_SELECTED_STATION);
  prntln(num);
  changed = true;
}

void Stations::deselect(int num) {
  if (!check(num)) return;
  internal_deselect(num);
  prnt(ST_DESELECTED_STATION);
  prntln(num);
  changed = true;
}

void Stations::selectAll() {
  for (int i = 0; i < count(); i++)
    internal_select(i);
  prntln(ST_SELECTED_ALL);
  changed = true;
}

void Stations::deselectAll() {
  for (int i = 0; i < count(); i++)
    internal_deselect(i);
  prntln(ST_DESELECTED_ALL);
  changed = true;
}

int Stations::count() {
  return list->size();
}

int Stations::selected() {
  int num = 0;
  for (int i = 0; i < count(); i++)
    if (getSelected(i)) num++;
  return num;
}

bool Stations::check(int num) {
  if (internal_check(num)){
    return true;
  } else{
    prnt(ST_ERROR_ID);
    prntln(num);
    return false;
  }
}

bool Stations::internal_check(int num) {
  return num >= 0 && num < list->size();
}

void Stations::internal_select(int num) {
  Station changedStation = list->get(num);
  changedStation.selected = true;
  list->set(num, changedStation);
}

void Stations::internal_deselect(int num) {
  Station changedStation = list->get(num);
  changedStation.selected = false;
  list->set(num, changedStation);
}

void Stations::internal_remove(int num) {
  free(getMac(num));
  free(getPkts(num));
  free(getTime(num));
  list->remove(num);
}

void Stations::internal_add(uint8_t* mac, int accesspointNum) {
  if (list->size() >= STATION_LIST_SIZE)
    removeOldest();

  Station newStation;
  newStation.ap = accesspointNum;
  newStation.ch = wifi_channel;
  newStation.mac = (uint8_t*)malloc(6);
  newStation.pkts = (uint32_t*)malloc(sizeof(uint32_t));
  newStation.time = (uint32_t*)malloc(sizeof(uint32_t));
  newStation.selected = false;

  memcpy(newStation.mac, mac, 6);
  *newStation.pkts = 1;
  *newStation.time = currentTime;

  list->add(newStation);
}

void Stations::internal_removeAll() {
  for (int i = 0; i < list->size(); i++) {
    free(getMac(i));
    free(getPkts(i));
    free(getTime(i));
  }
  list->clear();
}


