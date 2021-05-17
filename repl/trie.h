#pragma once

#include <optional>
#include <unordered_map>
#include <vector>

template <typename T> class Trie {
  using NodeID = size_t;

public:
  Trie() {
    CreateNode(0); // root
    for (char ch = 'a'; ch <= 'z'; ++ch) {
      CreateNode(0);
      next_[0][ch] = node_.size() - 1;
    }
    for (char ch = 'A'; ch <= 'Z'; ++ch) {
      CreateNode(0);
      next_[0][ch] = node_.size() - 1;
    }
  }

  void Insert(std::string k, T &&t) {
    NodeID cur = 0;
    for (auto ch : k) {
      if (!next_[cur].count(ch)) {
        CreateNode(cur);
        next_[cur][ch] = node_.size() - 1;
      }
      cur = next_[cur][ch];
    }
    node_[cur] = std::make_optional(t);
  }

  NodeID Next(size_t x, char ch) const {
    assert(HasEdge(x, ch));
    return next_[x].at(ch);
  }

  bool IsLeaf(NodeID x) const { return next_[x].empty(); }
  NodeID Parent(NodeID x) const { return parent_[x]; }
  bool Accepts(NodeID x) const { return node_[x].has_value(); }
  bool HasEdge(NodeID x, char ch) const { return next_[x].count(ch) > 0; }
  const T &Value(NodeID x) const { return node_[x].value(); }

private:
  std::vector<std::optional<T>> node_;
  std::vector<NodeID> parent_;
  std::vector<std::unordered_map<char, NodeID>> next_;

  void CreateNode(NodeID p) {
    node_.emplace_back(std::nullopt);
    parent_.emplace_back(p);
    next_.push_back({});
  }
};
