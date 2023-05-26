/** @file gsWeightMapper.h

    @brief Provides declaration of gsWeightMapper class.

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): A. Bressan, F. Buchegger
*/

#pragma once

#include <gsCore/gsExport.h>
#include <gsCore/gsLinearAlgebra.h>

namespace gismo
{

template<class T>
class gsWeightMapper
{
public:
    typedef T weightType;
    typedef index_t indexType; //indices of gsMatrix

    typedef gsSparseMatrix<weightType,Eigen::RowMajor,indexType> LToGMatrix;
    typedef gsSparseMatrix<weightType,Eigen::ColMajor,indexType> GToLMatrix;

    typedef std::vector<indexType> IndexContainer;
    typedef std::vector<indexType>::const_iterator CIndexIter;
    typedef std::vector<weightType> WeightContainer;
    typedef typename std::vector<weightType>::const_iterator CWeightIter;
    typedef typename LToGMatrix::InnerIterator InIterMat;
public:
    /**
     * @brief The Iterator struct
     * Provides fast read access to the mapper data. The implementation guarantees that no data is copied.
     * Can only be constructed from an optimized gsWeightMapper (see gsWeightMapper::optimize() ) by calling fastTargetToSource or fastSourceToTarget.
     *
     * Typical usage is
     * Iterator myIter=fastTargetToSource(targetId);
     * for( ; myIter ; ++myIter)
     * {
     *    myIter.index();   // source index
     *    myIter.weight();  // corresponding weight
     * }
     *
     * It should be compatible with the std::algoriths. The end marker can be obtained with the end() function.
     *
     * Note that any modification to the gsWeightMapper after the optimize call invalidates all iterators.
     */
    struct Iterator
    {
    friend class gsWeightMapper;
    private:
        const weightType  *m_beg;
        const weightType  *m_end;
        const weightType  *m_value;
        const indexType   *m_index;
        Iterator(const weightType  *beg,const weightType  *end,const indexType   *ind,const weightType  *value=NULL)
            : m_beg(beg), m_end(end), m_value(value?value:beg), m_index(ind)
        {}
        template<int ordering>
        Iterator( const gsSparseMatrix<weightType,ordering,indexType> &matrix, indexType outerId)
        {
            m_beg=matrix.valuePtr()+matrix.outerIndexPtr()[outerId];
            m_end=matrix.valuePtr()+matrix.outerIndexPtr()[outerId+1];
            m_value=m_beg;
            m_index=matrix.innerIndexPtr()+matrix.outerIndexPtr()[outerId];
        }
    public:
        Iterator()
            : m_beg(NULL),m_end(NULL),m_value(NULL),m_index(NULL)
            {
            }

        typedef std::bidirectional_iterator_tag iterator_category; //or another tag

        operator const weightType*() {return m_value;}
        operator const indexType*()  {return m_index;}

        // assignement shoud be implicitly generated by the compiler // Iterator& operator=(const Iterator& other) {m_value=other.m_value; m_index=other.m_index;  return *this;}
        bool operator==(const Iterator& other) const     {return m_value==other.m_value && m_index==other.m_index;}
        bool operator!=(const Iterator& other) const     {return !this->operator==(other);}

        Iterator& operator++()                     {++m_value;++m_index; return *this;}
        Iterator& operator--()                     {--m_value;--m_index; return *this;}
        Iterator& operator+=(ptrdiff_t a)          {m_value+=a; m_index+=a; return *this;}
        Iterator  operator+ (ptrdiff_t a) const    {return Iterator(m_beg,m_end, m_index+a, m_value+a);}
        Iterator& operator-=(ptrdiff_t a)          {m_value-=a; m_index-=a; return *this;}
        Iterator  operator- (ptrdiff_t a) const    {return Iterator(m_beg,m_end, m_index-a,m_value-a);}

        const weightType& operator*()    const     {return *m_value;}

        /**
         * @brief weight
         * @return the weight of the current entry
         */
        const weightType& weight()       const     {return *m_value;}
        /**
         * @brief index
         * @return the index of the current entry
         */
        const indexType&  index()        const     {return *m_index;}
        /**
         * @brief end
         * @return an iterator signaling the end to use in std::algorithm
         */
        Iterator   end()        const     {return Iterator(m_beg,m_end,m_index,m_end);}
        /**
         * @brief end
         * @return an iterator signaling the end to use in std::algorithm
         */
        Iterator  begin()      const     {return Iterator(m_beg,m_end,m_index,m_beg);}

        operator bool()        {return m_value<m_end && m_value>=m_beg;}

    };

    //////////////////////////////////////////////////
    // operators and constructors
    //////////////////////////////////////////////////
public:
    gsWeightMapper()
    { m_optimizationMatrix=NULL; }

    gsWeightMapper(indexType sourceSize,indexType targetSize)
    {
        m_matrix.resize(sourceSize,targetSize);
        m_optimizationMatrix=NULL;
    }

    virtual ~gsWeightMapper()
    {
        if(m_optimizationMatrix)
            delete m_optimizationMatrix;
    }

    gsWeightMapper(const gsWeightMapper& other)
        : m_matrix(other.m_matrix)
    {
        m_optimizationMatrix=NULL;
        optimize(other.getOptimizationFlags());
    }

    gsWeightMapper(const gsSparseMatrix<weightType,Eigen::RowMajor,indexType> & other)
    {
        m_optimizationMatrix=NULL;
        *this=other;
    }

    gsWeightMapper(const gsSparseMatrix<weightType,Eigen::ColMajor,indexType> & other)
    {
        m_optimizationMatrix=NULL;
        *this=other;
    }

    gsWeightMapper& operator=(const gsWeightMapper & other)
    {
        removeOptimization();
        m_matrix=other.m_matrix;
        optimize(other.getOptimizationFlags());
        return *this;
    }

    template<typename MatrixT>
    gsWeightMapper& operator*=(const MatrixT &other)
    {
        removeOptimization();
        // EIGEN problem
        // force a temporary because otherwise
        // permutation matrices do not work
        m_matrix=(m_matrix*other).eval();
        return *this;
    }

    template<typename MatrixT>
    gsWeightMapper operator*(const MatrixT &other)
    {
        gsWeightMapper result;
        result.m_matrix=m_matrix*other;
        return result;
    }

    template<typename MatrixT>
    gsWeightMapper& operator=(const MatrixT &other)
    {
        removeOptimization();
        m_matrix=other;
        optimize();
        return *this;
    }

    operator const LToGMatrix& ()
    { return m_matrix; }

    const LToGMatrix& asMatrix() const
    { return m_matrix; }

    LToGMatrix& asMatrix()
    { return m_matrix; }
    //////////////////////////////////////////////////
    // functions for working with the mapper
    //////////////////////////////////////////////////
public:

    /// sets the entry
    void setEntry(indexType source, indexType target, weightType weight=1)
    {
        removeOptimization();
        m_matrix.at(source,target)=weight;
    }

    /// gives back the weight of the \a source basisfunction when forming the \a target one
    weightType getWeight(indexType source, indexType target) const
    { return m_matrix.at(source,target); }

    /// gives back the amount of source basis functions in the mapping
    indexType getNrOfSources() const
    { return m_matrix.rows(); }

    /// gives back the amount of target basis functions in the mapping
    indexType getNrOfTargets() const
    { return m_matrix.cols(); }

    /// checks if the mapping for \a source is 1 to 1
    bool sourceIsId(indexType source) const
    {
        IndexContainer indices;
        sourceToTarget(source,indices);
        return (indices.size()==1 && math::almostEqual<14>(m_matrix.at(source,indices[0]),T(1.0)));
    }

    /// checks if the mapping for \a target is 1 to 1
    bool targetIsId(indexType target) const
    {
        IndexContainer indices;
        targetToSource(target,indices);
        return (indices.size()==1 && math::almostEqual<14>(m_matrix.at(indices[0],target),T(1.0)));
    }

    //////////////////////////////////////////////////
    // functions for transforming the coefficients
    //////////////////////////////////////////////////
public:

    /** Method to compute the coefficients of the patches from the target coefficients.
     *
     * \param[in] targetCoefs : same way stored as in gsGeometry
     * \param[out] sourceCoefs : same way stored as in gsGeometry
     */
    void mapToSourceCoefs(gsMatrix<weightType> const & targetCoefs,gsMatrix<weightType> & sourceCoefs) const
    {
        // from target to source it's just a multiplication
        sourceCoefs.noalias()=m_matrix * targetCoefs;
    }

    /** \brief Method to compute the target coefficients from the patch geometry coefficients
     *
     * If the source coefficients can not be converted to target ones without an error, a least
     * squares approximation is used to compute them.
     *
     * \param[in] sourceCoefs : same way stored as in gsGeometry
     * \param[out] targetCoefs : same way stored as in gsGeometry
     */
    void mapToTargetCoefs(gsMatrix<weightType> const & sourceCoefs,gsMatrix<weightType> & targetCoefs) const;

    //////////////////////////////////////////////////
    // functions for applying the map between target and source
    //////////////////////////////////////////////////
public:

    /** Takes an array of source basis functions (indexType) and gives back a vector with
     * all associated target basis functions. Duplicates will be removed.
     *
     * \param[in] source : array of indexType, source basis functions
     * \param[out] target : array of indexType, target basis functions
     */
    void sourceToTarget(IndexContainer const & source,IndexContainer & target) const
    {
        target.reserve( source.size() );
        target.clear();
        IndexContainer res_i;
        for(size_t i = 0;i<source.size();i++)
        {
            //just call sourceToTarget for single bfs for all given sources
            sourceToTarget(source[i],res_i);
            target.insert(target.end(), res_i.begin(), res_i.end());
        }
        //get rid of duplicates and sort them
        sort( target.begin(), target.end() );
        target.erase( unique( target.begin(), target.end() ), target.end() );
    }

    /** Takes an indexType source basisfunction and returns the
     * array of indexType, target basis functions that are associated with the
     * source one.
     *
     * \param[in] source : indexType, source basis function
     * \param[out] target : array of indexType, target basis functions
     */
    void sourceToTarget(indexType source,IndexContainer & target) const
    {
        WeightContainer weights;
        sourceToTarget(source,target,weights);
    }

    /** Takes an indexType source basisfunction and returns the
     * array of indexType, target basis functions that are associated with the
     * source one and a array of weightType, its associated weights.
     *
     * \param[in] source : indexType, source basis function
     * \param[out] target : array of indexType, target basis functions
     * \param[out] weights : array of weightType, target basis functions weights
     */
    void sourceToTarget(indexType source, IndexContainer & target, WeightContainer & weights) const;

    /** Takes an array of target basis functions (indexType) and gives back a vector with
     * all associated source basis functions. Duplicates will be removed.
     *
     * \param[in] target : array of indexType, target basis functions
     * \param[out] source : array of indexType, source basis functions
     */
    void targetToSource(IndexContainer const & target, IndexContainer & source) const
    {
        source.clear();
        IndexContainer res_i;
        for(size_t i = 0;i<target.size();i++)
        {
            //just call targetToSource for single bfs for all given targets
            targetToSource(target[i],res_i);
            source.insert(source.end(), res_i.begin(), res_i.end());
        }
        //get rid of duplicates and sort them
        sort( source.begin(), source.end() );
        source.erase( unique( source.begin(), source.end() ), source.end() );
    }

    /** Takes a target basis function (indexType) and gives back a vector with
     * all associated source basis functions.
     *
     * \param[in] target : indexType, target basis function
     * \param[out] source : array of indexType, source basis functions
     */
    void targetToSource(indexType target, IndexContainer & source) const
    {
        WeightContainer weights;
        targetToSource(target,source,weights);
    }

    /** Takes a target basis function (indexType) and gives back a vector with
     * all associated source basis functions.
     *
     * \param[in] target : indexType, target basis function
     * \param[out] source : array of indexType, source basis functions
     * \param[out] weights : array of weightType, source basis functions weights
     */
    void targetToSource(indexType target, IndexContainer & source, WeightContainer & weights) const;

    //////////////////////////////////////////////////
    // functions for fast access to the mapping data
    //////////////////////////////////////////////////
public:
    /**
     * @brief The finalizeFlags enum collets the flag accepted by the optimize function
     *
     *   fastSourceToTarget allows fast discovery of the target functions indeces and weights corresponding to a source function
     *   fastTargetToSource allows fast discovery of the source functions contributing to a target function
     *
     */
    enum optimizeFlags
    {
        optSourceToTarget = 1U<<0,
        optTargetToSource = 1U<<1
    };

    /**
     * @brief optimize
     * Prepare the mapper for fast read access.
     * This has some upfront cost, but can speed up later accesses.
     * It is required to call this function before calling fastSourceToTarget   fastTargetToSource   mapToTargetCoefs.
     *
     * Optimizing for sourceToTarget is the default and should improve both access time and memory consumption also while using the standard interface.
     * Optimizing for targetToSource is disabled by default because it doubles the memory consumption and it is rarely needed in critical code paths.
     * @param flags
     *
     * Note that any modification to the gsWeightMapper after the optimize call invalidates all iterators.
     */
    void optimize (size_t flags=optSourceToTarget) const
    {
        flags|= flags & optTargetToSource ? optSourceToTarget : 0;
        flags = flags & ~getOptimizationFlags();
        if(flags & optSourceToTarget)
        {
            m_matrix.prune(1,T(10)*std::numeric_limits<weightType>::epsilon());
            m_matrix.makeCompressed();
        }
        if(flags & optTargetToSource)
        {
            this->m_optimizationMatrix=new GToLMatrix();
            *m_optimizationMatrix=m_matrix;
            m_optimizationMatrix->prune(1,T(10)*std::numeric_limits<weightType>::epsilon());
            m_optimizationMatrix->makeCompressed();
        }
    }

    /// gives back the optimization flags
    size_t getOptimizationFlags() const
    {
        size_t flags=0;
        flags |= m_matrix.isCompressed() ? optSourceToTarget : 0;
        flags |= m_optimizationMatrix ? optTargetToSource : 0;
        return flags;
    }

    /**
     * @brief fastTargetToSource
     * @param  target the target index to look up
     * @return iterator  an iterator on the corresponding source indices (see gsWeightMapper::Iterator documentation)
     *
     * Note that any modification to the gsWeightMapper after the optimize call invalidates all iterators.     */
    Iterator fastTargetToSource(indexType target) const
    {
        GISMO_ASSERT(m_optimizationMatrix,"optimize() must be called on the mapper with fastTargetToSource flag before using this function.");
        GISMO_ASSERT(target<m_matrix.cols() && 0<=target,"index out of bounds");
        return Iterator(*m_optimizationMatrix,target);
    }
    /**
     * @brief fastSourceToTarget
     * @param  source    the source index to look up
     * @return iterator  an iterator on the corresponding target indices (see gsWeightMapper::Iterator documentation)
     *
     * Note that any modification to the gsWeightMapper after the optimize call invalidates all iterators.
     */
    Iterator fastSourceToTarget(indexType source) const
    {
        GISMO_ASSERT(m_matrix.isCompressed(),"optimize() must be called on the mapper with fastSourceToTarget flag before using this function.");
        GISMO_ASSERT(source<m_matrix.rows() && 0<=source,"index out of bounds");
        return Iterator(m_matrix,source);
    }


    /** Takes an array of source basis functions (indexType) and gives back a vector with
     * all associated target basis functions. Duplicates will be removed.
     *
     * \param[in] source : array of indexType, source basis functions
     * \param[out] target : array of indexType, target basis functions
     */
    void fastSourceToTarget(IndexContainer const & source,IndexContainer & target) const;

    /** Takes an array of target basis functions (indexType) and gives back a vector with
     * all associated source basis functions. Duplicates will be removed.
     *
     * \param[in]  target : array of indexType, target basis functions
     * \param[out] source : array of indexType, source basis functions
     */
    void fastTargetToSource(IndexContainer const & target,IndexContainer & source) const;


    /**
       @brief getLocalMap
       @param[in]  source : array of indexType, source basis functions
       @param[in]  target : array of indexType, target basis functions
       @param[out] map    : a matrix containing the coefficients of the expansion of the targets as
                            linear combination of the sources. Targets corresponds to columns, sources to rows.
     */
     void getLocalMap (IndexContainer const & source, IndexContainer const & target, gsMatrix<T> &map) const;

    /**
       @brief getLocalMap
       @param[in]  source : array of indexType, source basis functions
       @param[out] map    : a matrix containing the coefficinets of the expansion of all the targets as
                            linear combination of the sources. Targets corresponds to columns, sources to rows.
     */
     void getLocalMap (IndexContainer const & source, gsMatrix<T> &map) const;


private:
    void removeOptimization() {if(m_optimizationMatrix) delete m_optimizationMatrix; m_optimizationMatrix=NULL;}
    mutable GToLMatrix *m_optimizationMatrix;
    mutable LToGMatrix  m_matrix;
};//gsWeightMapper

}//namespace gismo

#ifndef GISMO_BUILD_LIB
#include GISMO_HPP_HEADER(gsWeightMapper.hpp)
#endif
