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
// Functions and classes to invert an FST.

#ifndef FST_INVERT_H_
#define FST_INVERT_H_

#include <cstdint>
#include <memory>

#include <fst/arc-map.h>
#include <fst/arc.h>
#include <fst/cache.h>
#include <fst/float-weight.h>
#include <fst/fst.h>
#include <fst/impl-to-fst.h>
#include <fst/mutable-fst.h>
#include <fst/properties.h>
#include <fst/symbol-table.h>

namespace fst {

// Mapper to implement inversion of an arc.
template <class A>
struct InvertMapper {
  using FromArc = A;
  using ToArc = A;

  InvertMapper() = default;

  ToArc operator()(const FromArc &arc) const {
    return ToArc(arc.olabel, arc.ilabel, arc.weight, arc.nextstate);
  }

  constexpr MapFinalAction FinalAction() const { return MAP_NO_SUPERFINAL; }

  constexpr MapSymbolsAction InputSymbolsAction() const {
    return MAP_CLEAR_SYMBOLS;
  }

  constexpr MapSymbolsAction OutputSymbolsAction() const {
    return MAP_CLEAR_SYMBOLS;
  }

  uint64_t Properties(uint64_t props) const { return InvertProperties(props); }
};

// Inverts the transduction corresponding to an FST by exchanging the
// FST's input and output labels.
//
// Complexity:
//
//   Time: O(V + E)
//   Space: O(1)
//
// where V is the number of states and E is the number of arcs.
template <class Arc>
inline void Invert(const Fst<Arc> &ifst, MutableFst<Arc> *ofst) {
  std::unique_ptr<SymbolTable> input(
      ifst.InputSymbols() ? ifst.InputSymbols()->Copy() : nullptr);
  std::unique_ptr<SymbolTable> output(
      ifst.OutputSymbols() ? ifst.OutputSymbols()->Copy() : nullptr);
  ArcMap(ifst, ofst, InvertMapper<Arc>());
  ofst->SetInputSymbols(output.get());
  ofst->SetOutputSymbols(input.get());
}

// Destructive variant of the above.
template <class Arc>
inline void Invert(MutableFst<Arc> *fst) {
  std::unique_ptr<SymbolTable> input(
      fst->InputSymbols() ? fst->InputSymbols()->Copy() : nullptr);
  std::unique_ptr<SymbolTable> output(
      fst->OutputSymbols() ? fst->OutputSymbols()->Copy() : nullptr);
  ArcMap(fst, InvertMapper<Arc>());
  fst->SetInputSymbols(output.get());
  fst->SetOutputSymbols(input.get());
}

// Inverts the transduction corresponding to an FST by exchanging the
// FST's input and output labels. This version is a delayed FST.
//
// Complexity:
//
//   Time: O(v + e)
//   Space: O(1)
//
// where v is the number of states visited and e is the number of arcs visited.
// Constant time and to visit an input state or arc is assumed and exclusive of
// caching.
template <class A>
class InvertFst : public ArcMapFst<A, A, InvertMapper<A>> {
  using Base = ArcMapFst<A, A, InvertMapper<A>>;

 public:
  using Arc = A;

  using Mapper = InvertMapper<Arc>;
  using typename Base::Impl;

  explicit InvertFst(const Fst<Arc> &fst) : Base(fst) {
    GetMutableImpl()->SetOutputSymbols(fst.InputSymbols());
    GetMutableImpl()->SetInputSymbols(fst.OutputSymbols());
  }

  // See Fst<>::Copy() for doc.
  InvertFst(const InvertFst &fst, bool safe = false) : Base(fst, safe) {}

  // Get a copy of this InvertFst. See Fst<>::Copy() for further doc.
  InvertFst *Copy(bool safe = false) const override {
    return new InvertFst(*this, safe);
  }

 private:
  using Base::GetMutableImpl;
};

// Specialization for InvertFst.
template <class Arc>
class StateIterator<InvertFst<Arc>>
    : public StateIterator<ArcMapFst<Arc, Arc, InvertMapper<Arc>>> {
 public:
  explicit StateIterator(const InvertFst<Arc> &fst)
      : StateIterator<ArcMapFst<Arc, Arc, InvertMapper<Arc>>>(fst) {}
};

// Specialization for InvertFst.
template <class Arc>
class ArcIterator<InvertFst<Arc>>
    : public ArcIterator<ArcMapFst<Arc, Arc, InvertMapper<Arc>>> {
 public:
  using StateId = typename Arc::StateId;

  ArcIterator(const InvertFst<Arc> &fst, StateId s)
      : ArcIterator<ArcMapFst<Arc, Arc, InvertMapper<Arc>>>(fst, s) {}
};

// Useful alias when using StdArc.
using StdInvertFst = InvertFst<StdArc>;

}  // namespace fst

#endif  // FST_INVERT_H_
