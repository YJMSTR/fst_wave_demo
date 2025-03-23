#include <cstdio>
#include <tuple>
#include "common.h"
class NodeElement;
bool compMergable(NodeElement* ele1, NodeElement* ele2);

enum ElementType { ELE_EMPTY, ELE_NODE, ELE_INT, ELE_SPACE};
enum OPLevel {OPL_BITS, OPL_LOGI, OPL_ARITH};
#define referNode(iter) (std::get<0>(iter))
#define referHi(iter) (std::get<1>(iter))
#define referLo(iter) (std::get<2>(iter))
#define referLevel(iter) (std::get<3>(iter))

class NodeElement {
public:
  ElementType eleType = ELE_EMPTY;
  Node* node = nullptr;
  mpz_t val;
  int hi, lo;
  std::set<std::tuple<Node*, int, int, OPLevel>> referNodes;
  OPLevel referType;
  NodeElement(ElementType type = ELE_EMPTY, Node* _node = nullptr, int _hi = -1, int _lo = -1) {
    mpz_init(val);
    eleType = type;
    node = _node;
    hi = _hi;
    lo = _lo;
    if (type == ELE_NODE) {
      referNodes.insert(std::make_tuple(_node, _hi, _lo, OPL_BITS));
    }
  }
  NodeElement(std::string str, int base = 16, int _hi = -1, int _lo = -1) {
    mpz_init(val);
    mpz_set_str(val, str.c_str(), base);
    hi = _hi;
    lo = _lo;
    eleType = ELE_INT;
    updateWidth();
  }
  void updateWidth() {
    if (lo != 0) mpz_tdiv_q_2exp(val, val, lo);
    mpz_t mask;
    mpz_init(mask);
    mpz_set_ui(mask, 1);
    mpz_mul_2exp(mask, mask, hi - lo + 1);
    mpz_sub_ui(mask, mask, 1);
    mpz_and(val, val, mask);
    hi = hi - lo;
    lo = 0;
  }
  NodeElement* dup() {
    NodeElement* ret = new NodeElement();
    mpz_set(ret->val, val);
    ret->eleType = eleType;
    ret->node = node;
    ret->hi = hi;
    ret->lo = lo;
    ret->referNodes.insert(referNodes.begin(), referNodes.end());
    return ret;
  }
  /* select [_hi, lo] */
  NodeElement* getBits(int _hi, int _lo) {
    Assert(_hi <= hi - lo + 1, "invalid range [%d, %d]", _hi, lo);
    NodeElement* ret = dup();
    if (eleType == ELE_NODE) {
      ret->hi = _hi + ret->lo;
      ret->lo = _lo + ret->lo;
    } else {
      ret->hi = _hi;
      ret->lo = _lo;
      if (eleType == ELE_INT) ret->updateWidth();
    }
    for (auto iter : ret->referNodes) {
      if (referLevel(iter) != OPL_ARITH) {
        referHi(iter) = std::get<2>(iter) + _hi;
        referLo(iter) = std::get<2>(iter) + _lo;
      }
    }
    return ret;
  }
  void merge(NodeElement* ele) {
    Assert(compMergable(this, ele), "not mergable");
    if (eleType == ELE_NODE) {
      lo = ele->lo;
      referNodes.clear();
      referNodes.insert(std::make_tuple(ele->node, hi, lo, OPL_BITS));
    } else if (eleType == ELE_INT) {
      mpz_mul_2exp(val, val, ele->hi - lo + 1);
      mpz_add(val, val, ele->val);
      hi += ele->hi + 1;
      updateWidth();
    }
  }
};

class NodeComponent{
public:
  std::vector<NodeElement*> elements;
  std::vector<NodeElement*> directElements;
  int width;
  NodeComponent() {
    width = 0;
  }
  void addElement(NodeElement* element) {
    elements.push_back(element);
    width += element->hi - element->lo + 1;
  }
  void addDirectElement(NodeElement* element) {
    directElements.push_back(element);
  }
  void addElementAll(NodeElement* element) {
    addElement(element);
    addDirectElement(element->dup());
  }
  void addfrontElement(NodeElement* element) {
    elements.insert(elements.begin(), element);
    width += element->hi - element->lo + 1;
  }
  void merge(NodeComponent* node) {
    if (elements.size() != 0 && node->elements.size() != 0 && compMergable(elements.back(), node->elements[0])) {
      elements.back()->merge(node->elements[0]);
      elements.insert(elements.end(), node->elements.begin() + 1, node->elements.end());
    } else {
      elements.insert(elements.end(), node->elements.begin(), node->elements.end());
    }
    if (directElements.size() != 0 && node->directElements.size() != 0 && compMergable(directElements.back(), node->directElements[0])) {
      directElements.back()->merge(node->directElements[0]);
      directElements.insert(directElements.end(), node->directElements.begin() + 1, node->directElements.end());
    } else {
      directElements.insert(directElements.end(), node->directElements.begin(), node->directElements.end());
    }
    width += node->countWidth();
  }
  NodeComponent* dup() {
    NodeComponent* ret = new NodeComponent();
    for (size_t i = 0; i < elements.size(); i ++) {
      ret->addElement(elements[i]->dup());
    }
    for (size_t i = 0; i < directElements.size(); i ++) {
      ret->addDirectElement(directElements[i]->dup());
    }
    return ret;
  }
  void getElementBits(NodeComponent* src, int hi, int lo) {
    int w = src->width;
    bool start = false;
    if (hi >= src->width) {
      addElement(new NodeElement("0", 16, hi - MAX(lo, src->width), 0));
      hi = MAX(lo, src->width - 1);
    }
    if (hi < src->width) {
      for (size_t i = 0; i < src->elements.size(); i ++) {
        int memberWidth = src->elements[i]->hi - src->elements[i]->lo + 1;
        if ((w > hi && (w - memberWidth) <= hi)) {
          start = true;
        }
        if (start) {
          int selectHigh = MIN(hi, w-1) - (w - memberWidth);
          int selectLo = MAX(lo, w - memberWidth) - (w - memberWidth);
          addElement(src->elements[i]->getBits(selectHigh, selectLo));
          if ((w - memberWidth) <= lo) break;
        }
        w = w - memberWidth;
      }
    }
  }
  void getDirectBits(NodeComponent* src, int hi, int lo) {
    int w = src->width;
    bool start = false;
    if (hi >= src->width) {
      addDirectElement(new NodeElement("0", 16, hi - MAX(lo, src->width), 0));
      hi = MAX(lo, src->width - 1);
    }
    if (hi < src->width) {
      for (size_t i = 0; i < src->directElements.size(); i ++) {
        int memberWidth = src->directElements[i]->hi - src->directElements[i]->lo + 1;
        if ((w > hi && (w - memberWidth) <= hi)) {
          start = true;
        }
        if (start) {
          int selectHigh = MIN(hi, w-1) - (w - memberWidth);
          int selectLo = MAX(lo, w - memberWidth) - (w - memberWidth);
          addDirectElement(src->directElements[i]->getBits(selectHigh, selectLo));
          if ((w - memberWidth) <= lo) break;
        }
        w = w - memberWidth;
      }
    }
  }
  NodeComponent* getbits(int hi, int lo) {
    NodeComponent* comp = new NodeComponent();
    comp->getElementBits(this, hi, lo);
    comp->getDirectBits(this, hi, lo);
    return comp;
  }
  int countWidth() {
    return width;
  }
  bool assignSegEq(NodeComponent* comp) {
    if (comp->width != width || comp->elements.size() != elements.size()) return false;
    for (size_t i = 0; i < elements.size(); i ++) {
      if ((elements[i]->hi - elements[i]->lo) != (comp->elements[i]->hi - comp->elements[i]->lo)) return false;
    }
    return true;
  }
  bool assignAllEq(NodeComponent* comp) {
    if (comp->width != width || comp->elements.size() != elements.size()) return false;
    for (size_t i = 0; i < elements.size(); i ++) {
      if ((elements[i]->hi - elements[i]->lo) != (comp->elements[i]->hi - comp->elements[i]->lo)) return false;
      if (elements[i]->eleType != comp->elements[i]->eleType) return false;
      if (elements[i]->eleType == ELE_NODE && elements[i]->node != comp->elements[i]->node) return false;
    }
    if (comp->directElements.size() != directElements.size()) return false;
    for (size_t i = 0; i < directElements.size(); i ++) {
      if ((directElements[i]->hi - directElements[i]->lo) != (comp->directElements[i]->hi - comp->directElements[i]->lo)) return false;
      if (directElements[i]->eleType != comp->directElements[i]->eleType) return false;
      if (directElements[i]->eleType == ELE_NODE && directElements[i]->node != comp->directElements[i]->node) return false;
    }
    return true;
  }
  void invalidateAll() {
    for (NodeElement* element : elements) {
      element->eleType = ELE_SPACE;
      element->hi -= element->lo;
      element->lo = 0;
      element->node = nullptr;
    }
  }
  void invalidateDirectAsWhole() {
    directElements.clear();
    directElements.push_back(new NodeElement(ELE_SPACE, nullptr, width - 1, 0));
  }
  bool fullValid() {
    if (elements.size() == 0) return false;
    for (NodeElement* element : elements) {
      if (element->eleType == ELE_SPACE) return false;
    }
    return true;
  }
  void setReferArith() {
    for (NodeElement* element : elements) {
      for (auto iter : element->referNodes) {
        referLevel(iter) = OPL_ARITH;
      }
    }
  }
  void setReferLogi() {
    for (NodeElement* element : elements) {
      for (auto iter : element->referNodes) {
        if (referLevel(iter) < OPL_LOGI) referLevel(iter) = OPL_LOGI;
      }
    }
  }
  void mergeNeighbor() {
    std::vector<NodeElement*> newElements;
    for (size_t i = 0; i < elements.size(); i ++) {
      if (newElements.size() != 0 && compMergable(newElements.back(), elements[i])) {
        newElements.back()->merge(elements[i]);
      } else newElements.push_back(elements[i]);
    }
    elements.clear();
    elements.insert(elements.end(), newElements.begin(), newElements.end());

    std::vector<NodeElement*> newDirectElements;
    for (size_t i = 0; i < directElements.size(); i ++) {
      if (newDirectElements.size() != 0 && compMergable(newDirectElements.back(), directElements[i])) {
        newDirectElements.back()->merge(directElements[i]);
      } else newDirectElements.push_back(directElements[i]);
    }
    directElements.clear();
    directElements.insert(directElements.end(), newDirectElements.begin(), newDirectElements.end());
  }
  void display() {
    printf("comp width %d\n", width);
    for (size_t i = 0; i < elements.size(); i ++) {
      NodeElement* element = elements[i];
      printf("=>  %s [%d, %d] (totalWidth = %d %p)\n", element->eleType == ELE_NODE ? element->node->name.c_str() : (element->eleType == ELE_INT ? (std::string("0x") + mpz_get_str(nullptr, 16, element->val)).c_str() : "EMPTY"),
            element->hi, element->lo, width, element);
      for (auto iter : element->referNodes) {
        printf("    -> %s [%d %d]\n", std::get<0>(iter)->name.c_str(), std::get<1>(iter), std::get<2>(iter));
      }
    }
    printf("==============\n");
    for (size_t i = 0; i < directElements.size(); i ++) {
      NodeElement* element = directElements[i];
      printf("=>  %s [%d, %d] (totalWidth = %d %p)\n", element->eleType == ELE_NODE ? element->node->name.c_str() : (element->eleType == ELE_INT ? (std::string("0x") + mpz_get_str(nullptr, 16, element->val)).c_str() : "EMPTY"),
            element->hi, element->lo, width, element);
      for (auto iter : element->referNodes) {
        printf("    -> %s [%d %d]\n", std::get<0>(iter)->name.c_str(), std::get<1>(iter), std::get<2>(iter));
      }
    }
  }
};

class Segments {
public:
  std::map<int, int> boundCount;
  std::map<int, int> concatCount;
  int width = 0;
  bool overlap = false;
  Segments(int _width) {
    width = _width;
  }
  Segments(NodeComponent* comp) {
    construct(comp);
  }
  void construct(NodeComponent* comp) {
    boundCount.clear();
    concatCount.clear();
    width = comp->width;
    int hi = comp->width - 1;
    for (NodeElement* element : comp->elements) {
      int nextHi = hi - (element->hi - element->lo + 1);
      addRange(hi, nextHi + 1, OPL_BITS);
      hi = nextHi;
    }
  }

  void addConcat(int idx, int count = 1) {
    if (idx >= width || idx < 0) return;
    if (concatCount.find(idx) == concatCount.end()) concatCount[idx] = 0;
    concatCount[idx] += count;
  }
  void eraseConcat(int idx, int count = 1) {
    if (idx >= width || idx < 0) return;
    Assert(concatCount.find(idx) != concatCount.end() || concatCount[idx] <= 0, "bound missing");
    concatCount[idx] -= count;
  }
  void addBound(int idx, int count = 1) {
    if (idx >= width || idx < 0) return;
    if (boundCount.find(idx) == boundCount.end()) boundCount[idx] = 0;
    boundCount[idx] += count;
  }
  void eraseBound(int idx, int count = 1) {
    if (idx >= width || idx < 0) return;
    Assert(boundCount.find(idx) != boundCount.end() || boundCount[idx] <= 0, "bound missing");
    boundCount[idx] -= count;
  }
  void addRange(int hi, int lo, OPLevel level) {
    addBound(hi);
    addBound(lo - 1);
    for (int i = lo; i < hi; i ++) addConcat(i);
  }
  void eraseRange(int hi, int lo, OPLevel level) {
    eraseBound(hi);
    eraseBound(lo - 1);
    for (int i = lo; i < hi; i ++) eraseConcat(i);
  }
};