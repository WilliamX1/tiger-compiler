#ifndef TIGER_UTIL_GRAPH_H_
#define TIGER_UTIL_GRAPH_H_

#include "tiger/util/table.h"

namespace graph {

template <typename T> class Node;
template <typename T> class NodeList;

template <typename T> class Graph {
public:
  // Make a new graph
  Graph() : nodecount_(0), my_nodes_(new NodeList<T>()) {}

  // Get the list of nodes belonging to graph
  NodeList<T> *Nodes();

  // Make a new node in graph "g", with associated "info_"
  Node<T> *NewNode(T *info);

  // Make a new edge joining nodes "from" and "to", which must belong
  // to the same graph
  void AddEdge(Node<T> *from, Node<T> *to);

  // Show all the nodes and edges in the graph, using the function "show_info"
  // to print the name of each node
  static void Show(FILE *out, NodeList<T> *p,
                   std::function<void(T *)> show_info);

  int nodecount_;

  ~Graph();

private:
  NodeList<T> *my_nodes_;
};

template <typename T> class Node {
  template <typename NodeType> friend class Graph;

public:
  // Tell if there is an edge from this node to "n"
  bool GoesTo(Node<T> *n);

  // Tell if node n is adjacent to this node
  bool Adj(Node<T> *n);

  // Return length of predecessor list for node n
  int InDegree();

  // Return length of successor list for node n
  int OutDegree();

  // Get all the successors and predecessors
  NodeList<T> *Adj();

  // Get all the successors of node
  NodeList<T> *Succ();

  // Get all the predecessors of node
  NodeList<T> *Pred();

  // Tell how many edges lead to or from node
  int Degree();

  // Get the "info_" associated with node
  T *NodeInfo();

  // Get the "my_key_" associated with node
  int Key();

  // dtors of Node
  ~Node<T>() {
    delete succs_;
    delete preds_;
  }

private:
  Graph<T> *my_graph_;
  int my_key_;
  NodeList<T> *succs_;
  NodeList<T> *preds_;
  T *info_;
  Node<T>()
      : my_graph_(nullptr), my_key_(0), succs_(nullptr), preds_(nullptr),
        info_(nullptr) {}
};

template <typename T> class NodeList {
  friend class Graph<T>;
  friend class Node<T>;

public:
  // Make a NodeList
  NodeList<T>() = default;
  ~NodeList<T>() = default;

  // Tell if "a" is in the list
  bool Contain(Node<T> *n);

  // Put list b at the back of list a and return the concatenated list
  void CatList(NodeList<T> *nl);
  void DeleteNode(Node<T> *n);
  void Clear() { node_list_.clear(); }
  void Prepend(Node<T> *n) { node_list_.push_front(n); }
  void Append(Node<T> *n) { node_list_.push_back(n); }

  // Set operation on two lists
  NodeList<T> *Union(NodeList<T> *nl);
  NodeList<T> *Diff(NodeList<T> *nl);

  [[nodiscard]] const std::list<Node<T> *> &GetList() const {
    return node_list_;
  }

private:
  std::list<Node<T> *> node_list_{};
};

// Generic creation of Node<tree>
template <typename T> Node<T> *Graph<T>::NewNode(T *info) {
  auto n = new Node<T>();
  n->my_graph_ = this;
  n->my_key_ = nodecount_++;

  my_nodes_->node_list_.push_back(n);

  n->succs_ = new NodeList<T>();
  n->preds_ = new NodeList<T>();
  n->info_ = info;

  return n;
}

template <typename T> bool Node<T>::GoesTo(Node<T> *n) {
  return succs_->Contain(n);
}

template <typename T> bool Node<T>::Adj(Node<T> *n) {
  return succs_->Contain(n) || preds_->Contain(n);
}

template <typename T> void Graph<T>::AddEdge(Node<T> *from, Node<T> *to) {
  assert(from);
  assert(to);
  assert(from->my_graph_ == this);
  assert(to->my_graph_ == this);
  if (from->GoesTo(to))
    return;
  to->preds_->node_list_.push_back(from);
  from->succs_->node_list_.push_back(to);
}

template <typename T> Graph<T>::~Graph() {
  for (auto node : my_nodes_->node_list_) {
    delete node;
  }
  delete my_nodes_;
}

template <typename T> int Node<T>::InDegree() {
  return preds_->node_list_.size();
}

template <typename T> int Node<T>::OutDegree() {
  return succs_->node_list_.size();
}

template <typename T> int Node<T>::Degree() { return InDegree() + OutDegree(); }

template <typename T> NodeList<T> *Node<T>::Adj() {
  NodeList<T> *adj_list = new NodeList<T>();
  adj_list->CatList(succs_);
  adj_list->CatList(preds_);
  return adj_list;
}

template <typename T> NodeList<T> *Node<T>::Succ() { return succs_; }

template <typename T> NodeList<T> *Node<T>::Pred() { return preds_; }

template <typename T> T *Node<T>::NodeInfo() { return info_; }

template <typename T> int Node<T>::Key() { return my_key_; }

template <typename T> NodeList<T> *Graph<T>::Nodes() { return my_nodes_; }

template <typename T> bool NodeList<T>::Contain(Node<T> *n) {
  for (auto node : node_list_) {
    if (node == n)
      return true;
  }
  return false;
}

template <typename T> void NodeList<T>::DeleteNode(Node<T> *n) {
  assert(n);
  auto it = node_list_.begin();
  for (; it != node_list_.end(); it++) {
    if (*it == n)
      break;
  }
  if (it == node_list_.end())
    return;
  node_list_.erase(it);
}

template <typename T> void NodeList<T>::CatList(NodeList<T> *nl) {
  if (!nl || nl->node_list_.empty())
    return;
  node_list_.insert(node_list_.end(), nl->node_list_.begin(),
                    nl->node_list_.end());
}

template <typename T> NodeList<T> *NodeList<T>::Union(NodeList<T> *nl) {
  NodeList<T> *res = new NodeList<T>();
  for (auto node : node_list_) {
    res->Append(node);
  }
  for (auto node : nl->GetList()) {
    if (!res->Contain(node))
      res->Append(node);
  }
  return res;
}

template <typename T> NodeList<T> *NodeList<T>::Diff(NodeList<T> *nl) {
  NodeList<T> *res = new NodeList<T>();
  for (auto node : node_list_) {
    if (!nl->Contain(node))
      res->node_list_.push_back(node);
  }
  return res;
}

// The type of "tables" mapping graph-nodes to information
template <typename T, typename ValueType>
using Table = tab::Table<Node<T>, ValueType>;

/**
 * Print a human-readable dump for debugging.
 * @tparam T node type
 * @param out output FILE object
 * @param p node_list_ to print
 * @param show_info show-information function
 */
template <typename T>
void Graph<T>::Show(FILE *out, NodeList<T> *p,
                    std::function<void(T *)> show_info) {
  for (Node<T> *n : p->node_list_) {
    assert(n);
    if (show_info)
      show_info(n->NodeInfo());
    fprintf(out, " (%d): ", n->Key());
    for (auto q : n->Succ()->node_list_)
      fprintf(out, "%d ", q->Key());
    for (auto q : n->Pred()->node_list_)
      fprintf(out, "%d ", q->Key());
    fprintf(out, "\n");
  }
}

} // namespace graph

#endif
