// Copyright 2005-2024 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the 'License');
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an 'AS IS' BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// See www.openfst.org for extensive documentation on this weighted
// finite-state transducer library.
//
// Expanded FST augmented with mutators; interface class definition and
// mutable arc iterator interface.

#ifndef FST_MUTABLE_FST_H_
#define FST_MUTABLE_FST_H_

#include <sys/types.h>

#include <cstddef>
#include <cstdint>
#include <ios>
#include <iostream>
#include <istream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <fst/log.h>
#include <fst/arc.h>
#include <fst/expanded-fst.h>
#include <fstream>
#include <fst/fst.h>
#include <fst/properties.h>
#include <fst/register.h>
#include <fst/symbol-table.h>
#include <string_view>

namespace fst {

template <class Arc>
struct MutableArcIteratorData;

// Abstract interface for an expanded FST which also supports mutation
// operations. To modify arcs, use MutableArcIterator.
template <class A>
class MutableFst : public ExpandedFst<A> {
 public:
  using Arc = A;
  using StateId = typename Arc::StateId;
  using Weight = typename Arc::Weight;

  virtual MutableFst<Arc> &operator=(const Fst<Arc> &fst) = 0;

  MutableFst &operator=(const MutableFst &fst) {
    return operator=(static_cast<const Fst<Arc> &>(fst));
  }

  // Sets the initial state.
  virtual void SetStart(StateId) = 0;

  // Sets a state's final weight.
  virtual void SetFinal(StateId s, Weight weight = Weight::One()) = 0;

  // Sets property bits w.r.t. mask.
  virtual void SetProperties(uint64_t props, uint64_t mask) = 0;

  // Adds a state and returns its ID.
  virtual StateId AddState() = 0;

  // Adds multiple states.
  virtual void AddStates(size_t) = 0;

  // Adds an arc to state.
  virtual void AddArc(StateId, const Arc &) = 0;

  // Adds an arc (passed by rvalue reference) to state. Allows subclasses
  // to optionally implement move semantics. Defaults to lvalue overload.
  virtual void AddArc(StateId state, Arc &&arc) { AddArc(state, arc); }

  // Deletes some states, preserving original StateId ordering.
  virtual void DeleteStates(const std::vector<StateId> &) = 0;

  // Delete all states.
  virtual void DeleteStates() = 0;

  // Delete some arcs at a given state.
  virtual void DeleteArcs(StateId, size_t) = 0;

  // Delete all arcs at a given state.
  virtual void DeleteArcs(StateId) = 0;

  // Optional, best effort only.
  virtual void ReserveStates(size_t) {}

  // Optional, best effort only.
  virtual void ReserveArcs(StateId, size_t) {}

  // Returns input label symbol table or nullptr if not specified.
  const SymbolTable *InputSymbols() const override = 0;

  // Returns output label symbol table or nullptr if not specified.
  const SymbolTable *OutputSymbols() const override = 0;

  // Returns input label symbol table or nullptr if not specified.
  virtual SymbolTable *MutableInputSymbols() = 0;

  // Returns output label symbol table or nullptr if not specified.
  virtual SymbolTable *MutableOutputSymbols() = 0;

  // Sets input label symbol table; pass nullptr to delete table.
  virtual void SetInputSymbols(const SymbolTable *isyms) = 0;

  // Sets output label symbol table; pass nullptr to delete table.
  virtual void SetOutputSymbols(const SymbolTable *osyms) = 0;

  // Gets a copy of this MutableFst. See Fst<>::Copy() for further doc.
  MutableFst *Copy(bool safe = false) const override = 0;

  // Reads a MutableFst from an input stream, returning nullptr on error.
  static MutableFst *Read(std::istream &strm, const FstReadOptions &opts) {
    FstReadOptions ropts(opts);
    FstHeader hdr;
    if (ropts.header) {
      hdr = *opts.header;
    } else {
      if (!hdr.Read(strm, opts.source)) return nullptr;
      ropts.header = &hdr;
    }
    if (!(hdr.Properties() & kMutable)) {
      LOG(ERROR) << "MutableFst::Read: Not a MutableFst: " << ropts.source;
      return nullptr;
    }
    const auto &fst_type = hdr.FstType();
    const auto reader = FstRegister<Arc>::GetRegister()->GetReader(fst_type);
    if (!reader) {
      LOG(ERROR) << "MutableFst::Read: Unknown FST type \"" << fst_type
                 << "\" (arc type = \"" << A::Type() << "\"): " << ropts.source;
      return nullptr;
    }
    auto *fst = reader(strm, ropts);
    if (!fst) return nullptr;
    return down_cast<MutableFst *>(fst);
  }

  // Reads a MutableFst from a file; returns nullptr on error. An empty
  // source results in reading from standard input. If convert is true,
  // convert to a mutable FST subclass (given by convert_type) in the case
  // that the input FST is non-mutable.
  static MutableFst *Read(const std::string &source, bool convert = false,
                          std::string_view convert_type = "vector") {
    if (convert == false) {
      if (!source.empty()) {
        std::ifstream strm(source,
                                std::ios_base::in | std::ios_base::binary);
        if (!strm) {
          LOG(ERROR) << "MutableFst::Read: Can't open file: " << source;
          return nullptr;
        }
        return Read(strm, FstReadOptions(source));
      } else {
        return Read(std::cin, FstReadOptions("standard input"));
      }
    } else {  // Converts to 'convert_type' if not mutable.
      std::unique_ptr<Fst<Arc>> ifst(Fst<Arc>::Read(source));
      if (!ifst) return nullptr;
      if (ifst->Properties(kMutable, false)) {
        return down_cast<MutableFst *>(ifst.release());
      } else {
        std::unique_ptr<Fst<Arc>> ofst(Convert(*ifst, convert_type));
        ifst.reset();
        if (!ofst) return nullptr;
        if (!ofst->Properties(kMutable, false)) {
          LOG(ERROR) << "MutableFst: Bad convert type: " << convert_type;
        }
        return down_cast<MutableFst *>(ofst.release());
      }
    }
  }

  // For generic mutuble arc iterator construction; not normally called
  // directly by users.
  virtual void InitMutableArcIterator(StateId s,
                                      MutableArcIteratorData<Arc> *data) = 0;
};

// Mutable arc iterator interface, templated on the Arc definition. This is
// used by mutable arc iterator specializations that are returned by the
// InitMutableArcIterator MutableFst method.
template <class Arc>
class MutableArcIteratorBase : public ArcIteratorBase<Arc> {
 public:
  // Sets current arc.
  virtual void SetValue(const Arc &) = 0;
};

template <class Arc>
struct MutableArcIteratorData {
  std::unique_ptr<MutableArcIteratorBase<Arc>> base;  // Specific iterator.
};

// Generic mutable arc iterator, templated on the FST definition; a wrapper
// around a pointer to a more specific one.
//
// Here is a typical use:
//
//   for (MutableArcIterator<StdFst> aiter(&fst, s);
//        !aiter.Done();
//         aiter.Next()) {
//     StdArc arc = aiter.Value();
//     arc.ilabel = 7;
//     aiter.SetValue(arc);
//     ...
//   }
//
// This version requires function calls.
template <class FST>
class MutableArcIterator {
 public:
  using Arc = typename FST::Arc;
  using StateId = typename Arc::StateId;

  MutableArcIterator(FST *fst, StateId s) {
    fst->InitMutableArcIterator(s, &data_);
  }

  bool Done() const { return data_.base->Done(); }

  const Arc &Value() const { return data_.base->Value(); }

  void Next() { data_.base->Next(); }

  size_t Position() const { return data_.base->Position(); }

  void Reset() { data_.base->Reset(); }

  void Seek(size_t a) { data_.base->Seek(a); }

  void SetValue(const Arc &arc) { data_.base->SetValue(arc); }

  uint8_t Flags() const { return data_.base->Flags(); }

  void SetFlags(uint8_t flags, uint8_t mask) {
    return data_.base->SetFlags(flags, mask);
  }

 private:
  MutableArcIteratorData<Arc> data_;

  MutableArcIterator(const MutableArcIterator &) = delete;
  MutableArcIterator &operator=(const MutableArcIterator &) = delete;
};

namespace internal {

// MutableFst<A> case: abstract methods.
template <class Arc>
inline typename Arc::Weight Final(const MutableFst<Arc> &fst,
                                  typename Arc::StateId s) {
  return fst.Final(s);
}

template <class Arc>
inline ssize_t NumArcs(const MutableFst<Arc> &fst, typename Arc::StateId s) {
  return fst.NumArcs(s);
}

template <class Arc>
inline ssize_t NumInputEpsilons(const MutableFst<Arc> &fst,
                                typename Arc::StateId s) {
  return fst.NumInputEpsilons(s);
}

template <class Arc>
inline ssize_t NumOutputEpsilons(const MutableFst<Arc> &fst,
                                 typename Arc::StateId s) {
  return fst.NumOutputEpsilons(s);
}

}  // namespace internal

// A useful alias when using StdArc.
using StdMutableFst = MutableFst<StdArc>;

// This is a helper class template useful for attaching a MutableFst interface
// to its implementation, handling reference counting and COW semantics.
template <class I, class FST = MutableFst<typename I::Arc>>
class ImplToMutableFst : public ImplToExpandedFst<I, FST> {
  using Base = ImplToExpandedFst<I, FST>;

 public:
  using Impl = I;
  using Arc = typename Impl::Arc;
  using StateId = typename Arc::StateId;
  using Weight = typename Arc::Weight;

  using Base::operator=;

  void SetStart(StateId s) override {
    MutateCheck();
    GetMutableImpl()->SetStart(s);
  }

  void SetFinal(StateId s, Weight weight = Weight::One()) override {
    MutateCheck();
    GetMutableImpl()->SetFinal(s, std::move(weight));
  }

  void SetProperties(uint64_t props, uint64_t mask) override {
    // Can skip mutate check if extrinsic properties don't change,
    // since it is then safe to update all (shallow) copies
    const auto exprops = kExtrinsicProperties & mask;
    if (GetImpl()->Properties(exprops) != (props & exprops)) MutateCheck();
    GetMutableImpl()->SetProperties(props, mask);
  }

  StateId AddState() override {
    MutateCheck();
    return GetMutableImpl()->AddState();
  }

  void AddStates(size_t n) override {
    MutateCheck();
    return GetMutableImpl()->AddStates(n);
  }

  void AddArc(StateId s, const Arc &arc) override {
    MutateCheck();
    GetMutableImpl()->AddArc(s, arc);
  }

  void AddArc(StateId s, Arc &&arc) override {
    MutateCheck();
    GetMutableImpl()->AddArc(s, std::forward<Arc>(arc));
  }

  void DeleteStates(const std::vector<StateId> &dstates) override {
    MutateCheck();
    GetMutableImpl()->DeleteStates(dstates);
  }

  void DeleteStates() override {
    if (!Unique()) {
      const auto *isymbols = GetImpl()->InputSymbols();
      const auto *osymbols = GetImpl()->OutputSymbols();
      SetImpl(std::make_shared<Impl>());
      GetMutableImpl()->SetInputSymbols(isymbols);
      GetMutableImpl()->SetOutputSymbols(osymbols);
    } else {
      GetMutableImpl()->DeleteStates();
    }
  }

  void DeleteArcs(StateId s, size_t n) override {
    MutateCheck();
    GetMutableImpl()->DeleteArcs(s, n);
  }

  void DeleteArcs(StateId s) override {
    MutateCheck();
    GetMutableImpl()->DeleteArcs(s);
  }

  void ReserveStates(size_t n) override {
    MutateCheck();
    GetMutableImpl()->ReserveStates(n);
  }

  void ReserveArcs(StateId s, size_t n) override {
    MutateCheck();
    GetMutableImpl()->ReserveArcs(s, n);
  }

  const SymbolTable *InputSymbols() const override {
    return GetImpl()->InputSymbols();
  }

  const SymbolTable *OutputSymbols() const override {
    return GetImpl()->OutputSymbols();
  }

  SymbolTable *MutableInputSymbols() override {
    MutateCheck();
    return GetMutableImpl()->InputSymbols();
  }

  SymbolTable *MutableOutputSymbols() override {
    MutateCheck();
    return GetMutableImpl()->OutputSymbols();
  }

  void SetInputSymbols(const SymbolTable *isyms) override {
    MutateCheck();
    GetMutableImpl()->SetInputSymbols(isyms);
  }

  void SetOutputSymbols(const SymbolTable *osyms) override {
    MutateCheck();
    GetMutableImpl()->SetOutputSymbols(osyms);
  }

 protected:
  using Base::GetImpl;
  using Base::GetMutableImpl;
  using Base::InputSymbols;
  using Base::SetImpl;
  using Base::Unique;

  explicit ImplToMutableFst(std::shared_ptr<Impl> impl) : Base(impl) {}

  ImplToMutableFst(const ImplToMutableFst &fst, bool safe) : Base(fst, safe) {}

  void MutateCheck() {
    if (!Unique()) SetImpl(std::make_shared<Impl>(*this));
  }
};

}  // namespace fst

#endif  // FST_MUTABLE_FST_H_
