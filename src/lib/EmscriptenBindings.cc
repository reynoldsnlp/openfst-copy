#include <emscripten/bind.h>
#include <fst/fstlib.h>

using namespace emscripten;

// Type aliases for commonly used FST types
using LogArc = fst::ArcTpl<fst::LogWeightTpl<float>>;
using LogVectorFst = fst::VectorFst<LogArc>;
using LogStateIterator = fst::StateIterator<LogVectorFst>;
using LogArcIterator = fst::ArcIterator<LogVectorFst>;

using TropicalArc = fst::ArcTpl<fst::TropicalWeightTpl<float>>;
using TropicalVectorFst = fst::VectorFst<TropicalArc>;
using TropicalStateIterator = fst::StateIterator<TropicalVectorFst>;
using TropicalArcIterator = fst::ArcIterator<TropicalVectorFst>;

// Vector state typedefs to match error messages
using LogVectorState = fst::VectorState<LogArc, std::allocator<LogArc>>;
using TropicalVectorState = fst::VectorState<TropicalArc, std::allocator<TropicalArc>>;

// Custom wrapper functions to handle construction of various types
LogStateIterator* createLogStateIterator(LogVectorFst* fst) {
    return new LogStateIterator(*fst);
}

LogArcIterator* createLogArcIterator(LogVectorFst* fst, fst::StdFst::StateId s) {
    return new LogArcIterator(*fst, s);
}

TropicalStateIterator* createTropicalStateIterator(TropicalVectorFst* fst) {
    return new TropicalStateIterator(*fst);
}

TropicalArcIterator* createTropicalArcIterator(TropicalVectorFst* fst, fst::StdFst::StateId s) {
    return new TropicalArcIterator(*fst, s);
}

// Helper function to create a LogArc
LogArc* createLogArc(fst::StdFst::StateId ilabel, fst::StdFst::StateId olabel,
                    float weight, fst::StdFst::StateId nextstate) {
    return new LogArc(ilabel, olabel, fst::LogWeightTpl<float>(weight), nextstate);
}

// Helper function to create a TropicalArc
TropicalArc* createTropicalArc(fst::StdFst::StateId ilabel, fst::StdFst::StateId olabel,
                             float weight, fst::StdFst::StateId nextstate) {
    return new TropicalArc(ilabel, olabel, fst::TropicalWeightTpl<float>(weight), nextstate);
}

// Wrapper functions for SetFinal to avoid overload resolution issues
void logVectorFstSetFinal(LogVectorFst* fst, fst::StdFst::StateId state, fst::LogWeightTpl<float> weight) {
    fst->SetFinal(state, weight);
}

void tropicalVectorFstSetFinal(TropicalVectorFst* fst, fst::StdFst::StateId state, fst::TropicalWeightTpl<float> weight) {
    fst->SetFinal(state, weight);
}

// Constants for FST implementation types
// These match the actual implementation types used in OpenFst
enum class FstImplementationType {
    VECTOR_FST_TYPE = 1,
    CONST_FST_TYPE = 2,
    COMPACT_FST_TYPE = 3
};

// Add arc functions to address undefined symbols
void logVectorFstAddArc(LogVectorFst* fst, fst::StdFst::StateId state, const LogArc& arc) {
    fst->AddArc(state, arc);
}

void tropicalVectorFstAddArc(TropicalVectorFst* fst, fst::StdFst::StateId state, const TropicalArc& arc) {
    fst->AddArc(state, arc);
}

// Ensure VectorFst constructor with empty VectorState is bound
LogVectorFst* createLogVectorFst() {
    return new LogVectorFst();
}

TropicalVectorFst* createTropicalVectorFst() {
    return new TropicalVectorFst();
}

// Explicit instantiation for weight classes
template class fst::LogWeightTpl<float>;
template class fst::TropicalWeightTpl<float>;
template class fst::ArcTpl<fst::LogWeightTpl<float>>;
template class fst::ArcTpl<fst::TropicalWeightTpl<float>>;
template class fst::VectorState<fst::ArcTpl<fst::LogWeightTpl<float>>, std::allocator<fst::ArcTpl<fst::LogWeightTpl<float>>>>;
template class fst::VectorState<fst::ArcTpl<fst::TropicalWeightTpl<float>>, std::allocator<fst::ArcTpl<fst::TropicalWeightTpl<float>>>>;
template class fst::VectorFst<fst::ArcTpl<fst::LogWeightTpl<float>>, fst::VectorState<fst::ArcTpl<fst::LogWeightTpl<float>>, std::allocator<fst::ArcTpl<fst::LogWeightTpl<float>>>>>;
template class fst::VectorFst<fst::ArcTpl<fst::TropicalWeightTpl<float>>, fst::VectorState<fst::ArcTpl<fst::TropicalWeightTpl<float>>, std::allocator<fst::ArcTpl<fst::TropicalWeightTpl<float>>>>>;
template class fst::StateIterator<fst::VectorFst<fst::ArcTpl<fst::LogWeightTpl<float>>, fst::VectorState<fst::ArcTpl<fst::LogWeightTpl<float>>, std::allocator<fst::ArcTpl<fst::LogWeightTpl<float>>>>>>;
template class fst::StateIterator<fst::VectorFst<fst::ArcTpl<fst::TropicalWeightTpl<float>>, fst::VectorState<fst::ArcTpl<fst::TropicalWeightTpl<float>>, std::allocator<fst::ArcTpl<fst::TropicalWeightTpl<float>>>>>>;
template class fst::ArcIterator<fst::VectorFst<fst::ArcTpl<fst::LogWeightTpl<float>>, fst::VectorState<fst::ArcTpl<fst::LogWeightTpl<float>>, std::allocator<fst::ArcTpl<fst::LogWeightTpl<float>>>>>>;
template class fst::ArcIterator<fst::VectorFst<fst::ArcTpl<fst::TropicalWeightTpl<float>>, fst::VectorState<fst::ArcTpl<fst::TropicalWeightTpl<float>>, std::allocator<fst::ArcTpl<fst::TropicalWeightTpl<float>>>>>>;

EMSCRIPTEN_BINDINGS(openfst_bindings) {
    // Bind the LogWeightTpl class
    class_<fst::LogWeightTpl<float>>("LogWeight")
        .constructor<float>()
        .constructor<>()
        ;

    // Bind the TropicalWeightTpl class
    class_<fst::TropicalWeightTpl<float>>("TropicalWeight")
        .constructor<float>()
        .constructor<>()
        ;

    // Bind ArcTpl for LogWeight
    class_<LogArc>("LogArc")
        .constructor<>()
        .property("ilabel", &LogArc::ilabel)
        .property("olabel", &LogArc::olabel)
        .property("weight", &LogArc::weight)
        .property("nextstate", &LogArc::nextstate)
        ;

    function("createLogArc", &createLogArc, allow_raw_pointers());

    // Bind ArcTpl for TropicalWeight
    class_<TropicalArc>("TropicalArc")
        .constructor<>()
        .property("ilabel", &TropicalArc::ilabel)
        .property("olabel", &TropicalArc::olabel)
        .property("weight", &TropicalArc::weight)
        .property("nextstate", &TropicalArc::nextstate)
        ;

    function("createTropicalArc", &createTropicalArc, allow_raw_pointers());

    // Bind VectorState classes to fix undefined symbols
    class_<LogVectorState>("LogVectorState");
    class_<TropicalVectorState>("TropicalVectorState");

    // Bind the VectorFst class with LogWeight
    class_<LogVectorFst>("LogVectorFst")
        .constructor<>()
        .function("Start", &LogVectorFst::Start)
        .function("SetStart", &LogVectorFst::SetStart)
        .function("AddState", &LogVectorFst::AddState)
        .function("SetFinal", &logVectorFstSetFinal, allow_raw_pointers())
        .function("AddArc", &logVectorFstAddArc, allow_raw_pointers())
        .function("Final", &LogVectorFst::Final)
        .function("NumStates", &LogVectorFst::NumStates)
        .function("NumArcs", &LogVectorFst::NumArcs)
        ;

    function("createLogVectorFst", &createLogVectorFst, allow_raw_pointers());

    // Bind the VectorFst class with TropicalWeight
    class_<TropicalVectorFst>("TropicalVectorFst")
        .constructor<>()
        .function("Start", &TropicalVectorFst::Start)
        .function("SetStart", &TropicalVectorFst::SetStart)
        .function("AddState", &TropicalVectorFst::AddState)
        .function("SetFinal", &tropicalVectorFstSetFinal, allow_raw_pointers())
        .function("AddArc", &tropicalVectorFstAddArc, allow_raw_pointers())
        .function("Final", &TropicalVectorFst::Final)
        .function("NumStates", &TropicalVectorFst::NumStates)
        .function("NumArcs", &TropicalVectorFst::NumArcs)
        ;

    function("createTropicalVectorFst", &createTropicalVectorFst, allow_raw_pointers());

    // Bind the StateIterator classes
    class_<LogStateIterator>("LogStateIterator")
        .constructor<const LogVectorFst&>()
        .function("Done", &LogStateIterator::Done)
        .function("Value", &LogStateIterator::Value)
        .function("Next", &LogStateIterator::Next)
        .function("Reset", &LogStateIterator::Reset)
        ;

    // Bind generic StateIterator template
    class_<fst::StateIteratorBase<LogVectorFst>>("LogStateIteratorBase");
    class_<fst::StateIteratorBase<TropicalVectorFst>>("TropicalStateIteratorBase");

    // Factory function for LogStateIterator
    function("createLogStateIterator", &createLogStateIterator, allow_raw_pointers());

    // Bind the ArcIterator classes
    class_<LogArcIterator>("LogArcIterator")
        .constructor<const LogVectorFst&, fst::StdFst::StateId>()
        .function("Done", &LogArcIterator::Done)
        .function("Value", &LogArcIterator::Value)
        .function("Next", &LogArcIterator::Next)
        .function("Reset", &LogArcIterator::Reset)
        ;

    // Bind generic ArcIterator template bases
    class_<fst::ArcIteratorBase<LogVectorFst>>("LogArcIteratorBase");
    class_<fst::ArcIteratorBase<TropicalVectorFst>>("TropicalArcIteratorBase");

    // Factory function for LogArcIterator
    function("createLogArcIterator", &createLogArcIterator, allow_raw_pointers());

    // Bind the StateIterator classes for Tropical weights
    class_<TropicalStateIterator>("TropicalStateIterator")
        .constructor<const TropicalVectorFst&>()
        .function("Done", &TropicalStateIterator::Done)
        .function("Value", &TropicalStateIterator::Value)
        .function("Next", &TropicalStateIterator::Next)
        .function("Reset", &TropicalStateIterator::Reset)
        ;

    // Factory function for TropicalStateIterator
    function("createTropicalStateIterator", &createTropicalStateIterator, allow_raw_pointers());

    // Bind the ArcIterator classes for Tropical weights
    class_<TropicalArcIterator>("TropicalArcIterator")
        .constructor<const TropicalVectorFst&, fst::StdFst::StateId>()
        .function("Done", &TropicalArcIterator::Done)
        .function("Value", &TropicalArcIterator::Value)
        .function("Next", &TropicalArcIterator::Next)
        .function("Reset", &TropicalArcIterator::Reset)
        ;

    // Factory function for TropicalArcIterator
    function("createTropicalArcIterator", &createTropicalArcIterator, allow_raw_pointers());

    // Bind our FST implementation type constants
    enum_<FstImplementationType>("FstImplementationType")
        .value("VECTOR_FST_TYPE", FstImplementationType::VECTOR_FST_TYPE)
        .value("CONST_FST_TYPE", FstImplementationType::CONST_FST_TYPE)
        .value("COMPACT_FST_TYPE", FstImplementationType::COMPACT_FST_TYPE)
        ;
}
