/** @file gsAdaptiveRefUtils.h

    @brief Provides class for adaptive refinement.

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): H.M. Verhelst (TU Delft, 2019-)
*/

#pragma once


#include <iostream>
#include <gsAssembler/gsAdaptiveRefUtils.h>
#include <gsAssembler/gsAdaptiveMeshingCompare.h>
#include <gsIO/gsOptionList.h>
#include <gsCore/gsMultiPatch.h>
#include <gsCore/gsMultiBasis.h>
#include <gsHSplines/gsHBox.h>
#include <gsHSplines/gsHBoxContainer.h>
#include <gsHSplines/gsHBoxUtils.h>

namespace gismo
{

// enum MarkingStrategy
// {
//     GARU=1,
//     PUCA=2,
//     BULK=3
// };

template <class T>
class gsAdaptiveMeshing
{
public:
    typedef          gsHBox<2,T>                                HBox;
    typedef          gsHBox<2,T> *                              HBox_ptr;
    typedef          gsHBoxContainer<2,T>                       HBoxContainer;
    typedef typename HBox::SortedContainer                      boxContainer;
    typedef          std::map<gsHBox<2,T>,index_t,gsHBoxCompare<2,T>>  indexMapType;
    typedef          std::map<index_t,gsHBox<2,T>*>                 boxMapType;
    typedef          gsHBoxUtils<2,T>                 HBoxUtils;

public:

    gsAdaptiveMeshing(gsFunctionSet<T> & input);

    // ~gsAdaptiveMeshing();

    gsOptionList & options() {return m_options;}

    void defaultOptions();

    void getOptions();

    void rebuild();


    void markRef_into(const std::vector<T> & elError, HBoxContainer & elMarked);

    void markCrs_into(const std::vector<T> & elError, const HBoxContainer & markedRef, HBoxContainer & elMarked);
    void markCrs_into(const std::vector<T> & elError, HBoxContainer & elMarked);

    void markRef(const std::vector<T> & errors);
    void markCrs(const std::vector<T> & errors);

    bool refine(const HBoxContainer & markedRef);
    bool unrefine(const HBoxContainer & markedCrs);

    bool refine(const std::vector<bool> & markedRef) { return refine(_toContainer(markedRef)); }
    bool unrefine(const std::vector<bool> & markedCrs) { return unrefine(_toContainer(markedCrs)); };

    bool refine() { return refine(m_markedRef); }
    bool unrefine() { return unrefine(m_markedRef); };


    // void flatten(const index_t level);
    // void flatten() { flatten(m_maxLvl); } ;

    // void unrefineThreshold(const index_t level);
    // void unrefineThreshold(){ unrefineThreshold(m_maxLvl); };



private:
    void _makeMap(const gsFunctionSet<T> * input, typename gsAdaptiveMeshing<T>::indexMapType & indexMap, typename gsAdaptiveMeshing<T>::boxMapType & boxMap);

    void _assignErrors(boxMapType & container, const std::vector<T> & elError);


    void _refineMarkedElements(     const HBoxContainer & container,
                                    index_t refExtension = 0);

    void _unrefineMarkedElements(   const HBoxContainer & container,
                                    index_t refExtension = 0);

    // void _flattenElementsToLevel(   const index_t level);

    // void _unrefineElementsThreshold(const index_t level);

    std::vector<index_t> _sortPermutation( const boxMapType & container);
    // void _sortPermutated( const std::vector<index_t> & permutation, boxContainer & container);

    void _crsPredicates_into( std::vector<gsHBoxCheck<2,T> *> & predicates);
    void _crsPredicates_into(const HBoxContainer & markedRef, std::vector<gsHBoxCheck<2,T> *> & predicates);
    void _refPredicates_into( std::vector<gsHBoxCheck<2,T> *> & predicates);

    template<bool _coarsen,bool _admissible>
    void _markElements(  const std::vector<T> & elError, const index_t refCriterion, const T refParameter, const std::vector<index_t> & permutation, const std::vector<gsHBoxCheck<2,T> *> & predicates, HBoxContainer & elMarked);

    template<bool _coarsen,bool _admissible>
    void _markFraction( const boxMapType & elements, const T refParameter, const std::vector<gsHBoxCheck<2,T> *> predicates, const std::vector<index_t> & permutation, HBoxContainer & elMarked)
    {
        _markFraction_impl<_coarsen,_admissible>(elements,refParameter,predicates,permutation,elMarked);
    }
    template<bool _coarsen,bool _admissible>
    typename std::enable_if< _coarsen &&  _admissible, void>::type
    _markFraction_impl( const boxMapType & elements, const T refParameter, const std::vector<gsHBoxCheck<2,T> *> predicates, const std::vector<index_t> & permutation, HBoxContainer & elMarked);

    template<bool _coarsen,bool _admissible>
    typename std::enable_if< _coarsen && !_admissible, void>::type
    _markFraction_impl( const boxMapType & elements, const T refParameter, const std::vector<gsHBoxCheck<2,T> *> predicates, const std::vector<index_t> & permutation, HBoxContainer & elMarked);

    template<bool _coarsen,bool _admissible>
    typename std::enable_if<!_coarsen &&  _admissible, void>::type
    _markFraction_impl( const boxMapType & elements, const T refParameter, const std::vector<gsHBoxCheck<2,T> *> predicates, const std::vector<index_t> & permutation, HBoxContainer & elMarked);

    template<bool _coarsen,bool _admissible>
    typename std::enable_if<!_coarsen && !_admissible, void>::type
    _markFraction_impl( const boxMapType & elements, const T refParameter, const std::vector<gsHBoxCheck<2,T> *> predicates, const std::vector<index_t> & permutation, HBoxContainer & elMarked);

    template<bool _coarsen,bool _admissible>
    void _markPercentage( const boxMapType & elements, const T refParameter, const std::vector<gsHBoxCheck<2,T> *> predicates, const std::vector<index_t> & permutation, HBoxContainer & elMarked)
    {
        _markPercentage_impl<_coarsen,_admissible>(elements,refParameter,predicates,permutation,elMarked);
    }
    template<bool _coarsen,bool _admissible>
    typename std::enable_if< _coarsen &&  _admissible, void>::type
    _markPercentage_impl( const boxMapType & elements, const T refParameter, const std::vector<gsHBoxCheck<2,T> *> predicates, const std::vector<index_t> & permutation, HBoxContainer & elMarked);

    template<bool _coarsen,bool _admissible>
    typename std::enable_if< _coarsen && !_admissible, void>::type
    _markPercentage_impl( const boxMapType & elements, const T refParameter, const std::vector<gsHBoxCheck<2,T> *> predicates, const std::vector<index_t> & permutation, HBoxContainer & elMarked);

    template<bool _coarsen,bool _admissible>
    typename std::enable_if<!_coarsen &&  _admissible, void>::type
    _markPercentage_impl( const boxMapType & elements, const T refParameter, const std::vector<gsHBoxCheck<2,T> *> predicates, const std::vector<index_t> & permutation, HBoxContainer & elMarked);

    template<bool _coarsen,bool _admissible>
    typename std::enable_if<!_coarsen && !_admissible, void>::type
    _markPercentage_impl( const boxMapType & elements, const T refParameter, const std::vector<gsHBoxCheck<2,T> *> predicates, const std::vector<index_t> & permutation, HBoxContainer & elMarked);

    template<bool _coarsen,bool _admissible>
    void _markThreshold( const boxMapType & elements, const T refParameter, const std::vector<gsHBoxCheck<2,T> *> predicates, const std::vector<index_t> & permutation, HBoxContainer & elMarked)
    {
        _markThreshold_impl<_coarsen,_admissible>(elements,refParameter,predicates,permutation,elMarked);
    }
    template<bool _coarsen,bool _admissible>
    typename std::enable_if< _coarsen &&  _admissible, void>::type
    _markThreshold_impl( const boxMapType & elements, const T refParameter, const std::vector<gsHBoxCheck<2,T> *> predicates, const std::vector<index_t> & permutation, typename gsAdaptiveMeshing<T>::HBoxContainer & elMarked);

    template<bool _coarsen,bool _admissible>
    typename std::enable_if< _coarsen && !_admissible, void>::type
    _markThreshold_impl( const boxMapType & elements, const T refParameter, const std::vector<gsHBoxCheck<2,T> *> predicates, const std::vector<index_t> & permutation, typename gsAdaptiveMeshing<T>::HBoxContainer & elMarked);

    template<bool _coarsen,bool _admissible>
    typename std::enable_if<!_coarsen &&  _admissible, void>::type
    _markThreshold_impl( const boxMapType & elements, const T refParameter, const std::vector<gsHBoxCheck<2,T> *> predicates, const std::vector<index_t> & permutation, typename gsAdaptiveMeshing<T>::HBoxContainer & elMarked);

    template<bool _coarsen,bool _admissible>
    typename std::enable_if<!_coarsen && !_admissible, void>::type
    _markThreshold_impl( const boxMapType & elements, const T refParameter, const std::vector<gsHBoxCheck<2,T> *> predicates, const std::vector<index_t> & permutation, typename gsAdaptiveMeshing<T>::HBoxContainer & elMarked);

    bool _checkBox  ( const          HBox            & box  , const std::vector<gsHBoxCheck<2,T> *> predicates);
    bool _checkBoxes( const typename HBox::Container & boxes, const std::vector<gsHBoxCheck<2,T> *> predicates);

    T _totalError(const boxMapType & elements);

    T _maxError(  const boxMapType & elements);

    void _addAndMark(          HBox            & box  , typename gsAdaptiveMeshing<T>::HBoxContainer & elMarked);
    void _addAndMark( typename HBox::Container & boxes, typename gsAdaptiveMeshing<T>::HBoxContainer & elMarked);

    void _setContainerProperties( typename HBox::Container & boxes );

    // void _markLevelThreshold( index_t level, HBoxContainer & elMarked);
    void _getElLevels( std::vector<index_t> & elLevels);

    void _printMarking(const HBoxContainer & elMarked);

    HBox * _boxPtr(const HBox & box);

    typename gsAdaptiveMeshing<T>::HBoxContainer _toContainer( const std::vector<bool> & bools);

protected:
    // M & m_basis;
    gsFunctionSet<T> * m_input;
    // const gsMultiPatch<T> & m_patches;
    gsOptionList m_options;

    T               m_crsParam, m_refParam;
    MarkingStrategy m_crsRule, m_refRule;
    index_t         m_crsExt, m_refExt;
    index_t         m_maxLvl;

    index_t m_m;

    bool            m_admissible;

    index_t         m_verbose;

    HBoxContainer m_markedRef, m_markedCrs;
    // m_boxes is a container that does not contain patch IDs

    indexMapType m_indices;
    boxMapType   m_boxes;

    /*
        The plan:
            Make std::map<box,index> m_indices
            Make std::map<index,box*> m_boxes
        Which can be used to obtain the index of a box via m_indices[box] = index
        And to obtain the box corresponding to an index m_boxes[index] = *box
        THe latter can be used to obtain the neighborhood etc.

     */

    // std::map<index_t,std::shared_ptr<gsHBox<d,T>>> m_toindices;
    // std::map<std::shared_ptr<gsHBox<d,T>>,index_t> m_fromindices;


};

} // namespace gismo

#ifndef GISMO_BUILD_LIB
#include GISMO_HPP_HEADER(gsAdaptiveMeshing.hpp)
#else
#ifdef gsAdaptiveMeshing_EXPORT
#include GISMO_HPP_HEADER(gsAdaptiveMeshing.hpp)
#undef  EXTERN_CLASS_TEMPLATE
#define EXTERN_CLASS_TEMPLATE CLASS_TEMPLATE_INST
#endif
namespace gismo
{
    EXTERN_CLASS_TEMPLATE gsAdaptiveMeshing<real_t>;
    // EXTERN_CLASS_TEMPLATE gsHBoxCheck<1,real_t>;
    // EXTERN_CLASS_TEMPLATE gsHBoxCheck<2,real_t>;
    // EXTERN_CLASS_TEMPLATE gsHBoxCheck<3,real_t>;
    // EXTERN_CLASS_TEMPLATE gsHBoxCheck<4,real_t>;

    // EXTERN_CLASS_TEMPLATE gsLvlCompare<1,real_t>;
    // EXTERN_CLASS_TEMPLATE gsLvlCompare<2,real_t>;
    // EXTERN_CLASS_TEMPLATE gsLvlCompare<3,real_t>;
    // EXTERN_CLASS_TEMPLATE gsLvlCompare<4,real_t>;

    // EXTERN_CLASS_TEMPLATE gsSmallerErrCompare<1,real_t>;
    // EXTERN_CLASS_TEMPLATE gsSmallerErrCompare<2,real_t>;
    // EXTERN_CLASS_TEMPLATE gsSmallerErrCompare<3,real_t>;
    // EXTERN_CLASS_TEMPLATE gsSmallerErrCompare<4,real_t>;

    // EXTERN_CLASS_TEMPLATE gsLargerErrCompare<1,real_t>;
    // EXTERN_CLASS_TEMPLATE gsLargerErrCompare<2,real_t>;
    // EXTERN_CLASS_TEMPLATE gsLargerErrCompare<3,real_t>;
    // EXTERN_CLASS_TEMPLATE gsLargerErrCompare<4,real_t>;

    // EXTERN_CLASS_TEMPLATE gsOverlapCompare<1,real_t>;
    // EXTERN_CLASS_TEMPLATE gsOverlapCompare<2,real_t>;
    // EXTERN_CLASS_TEMPLATE gsOverlapCompare<3,real_t>;
    // EXTERN_CLASS_TEMPLATE gsOverlapCompare<4,real_t>;
}
#endif