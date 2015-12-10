#ifndef BPHRecoBuilder_H
#define BPHRecoBuilder_H
/** \class BPHRecoBuilder
 *
 *  Description: 
 *     Class to build all the combinations of decay products
 *     starting from reco::Candidate collections and applying
 *     selection cuts to decay products and reconstructed candidates
 *
 *  $Date: 2015-07-06 11:20:00 $
 *  $Revision: 1.1 $
 *  \author Paolo Ronchese INFN Padova
 *
 */

//----------------------
// Base Class Headers --
//----------------------


//------------------------------------
// Collaborating Class Declarations --
//------------------------------------
#include "BPHAnalysis/RecoDecay/interface/BPHDecayMomentum.h"
#include "BPHAnalysis/RecoDecay/interface/BPHDecayVertex.h"
#include "BPHAnalysis/RecoDecay/interface/BPHKinematicFit.h"
class BPHRecoSelect;
class BPHMomentumSelect;
class BPHVertexSelect;
class BPHFitSelect;

namespace edm {
  class EventSetup;
}

namespace reco {
  class RecoCandidate;
}

//---------------
// C++ Headers --
//---------------
#include <string>
#include <vector>
#include <map>
#include <set>


//              ---------------------
//              -- Class Interface --
//              ---------------------

class BPHRecoBuilder {

  friend class BPHRecoSelect;

 public:

  /** Constructor
   */
  BPHRecoBuilder( const edm::EventSetup& es );

  /** Destructor
   */
  virtual ~BPHRecoBuilder();

  /** Operations
   */

  // common object to interface with edm collections
  struct BPHGenericCollection {
    virtual ~BPHGenericCollection() {}
    virtual const reco::Candidate& get( int i ) const = 0;
    virtual int size() const = 0;
  };
  template <class T>
  static BPHGenericCollection* createCollection(
                               const edm::Handle<T>& collection );
  static BPHGenericCollection* createCollection(
                               const std::vector<const reco::Candidate*>&
                               candList );

  /// add collection of particles giving them a name
  /// collections can be added as 
  /// - for simple particles as edm::Handle of an edm collection
  ///   (an object with an operator [] returning a pointer to reco::Candidate)
  ///   or a BPHGenericCollection created from it
  /// - for previously reconstructed particles as std::vector
  ///   of pointers to BPHRecoCandidate or objects inheriting 
  ///   from BPHRecoCandidate
  void add( const std::string& name,
            const BPHGenericCollection* collection,
            double mass = -1.0,
            double msig = -1.0 );
  template <class T>
  void add( const std::string& name,
            const edm::Handle<T>& collection,
            double mass = -1.0,
            double msig = -1.0 );
  void add( const std::string& name,
            const std::vector<const BPHRecoCandidate*>& collection );
  template <class T>
  void add( const std::string& name,
            const std::vector<const T*>& collection );

  /// define selections to particles to be used in the reconstruction:
  /// simple particles
  void filter( const std::string& name, const BPHRecoSelect    & sel ) const;
  /// previously reconstructed particles, at simple momentum sum level
  void filter( const std::string& name, const BPHMomentumSelect& sel ) const;
  /// previously reconstructed particles, at vertex reconstruction level
  void filter( const std::string& name, const BPHVertexSelect  & sel ) const;
  /// previously reconstructed particles, at kinematical fit level
  void filter( const std::string& name, const BPHFitSelect     & sel ) const;

  /// define selections to recontructed particles, at different levels:
  /// simple momentum sum
  void filter( const BPHMomentumSelect& sel );
  /// vertex reconstruction
  void filter( const BPHVertexSelect& sel );
  /// kinematical fit
  void filter( const BPHFitSelect& sel );
  // apply selection to reconstructed candidate
  bool accept( const BPHRecoCandidate& cand ) const;

  /// define a min. squared momentum difference between two particles
  /// (relative difference to squared momentum sum is used)
  void setMinPDiffererence( double pMin );

  /// object to contain a combination of simple and previously reconstructed
  /// particles with their names
  struct ComponentSet {
    std::map<std::string,BPHDecayMomentum::Component> daugMap;
    std::map<std::string,const BPHRecoCandidate*>     compMap;
  };

  /// build a set of combinations of particles fulfilling the selections
  std::vector<ComponentSet> build() const;

  /// get the EventSetup set in the constructor
  const edm::EventSetup* eventSetup() const;

 private:

  // private copy and assigment constructors
  BPHRecoBuilder           ( const BPHRecoBuilder& x );
  BPHRecoBuilder& operator=( const BPHRecoBuilder& x );

  // object to interface with a specific edm collection
  template <class T>
  class BPHSpecificCollection: public BPHGenericCollection {
   public:
    BPHSpecificCollection( const T& c ): cPtr( &c ) {}
    virtual ~BPHSpecificCollection() {}
    virtual const reco::Candidate& get( int i ) const { return (*cPtr)[i]; }
    virtual int size() const { return cPtr->size(); }
   private:
    const T* cPtr;
  };

  // object to contain a list of simple particles
  // with their names, selections, masses and sigma
  struct BPHRecoSource {
    const std::string* name;
    const BPHGenericCollection* collection;
    std::vector<const BPHRecoSelect*> selector;
    double mass;
    double msig;
  };

  // object to contain a list of previously reconstructed particles
  // with their names and selections
  struct BPHCompSource {
    const std::string* name;
    const std::vector<const BPHRecoCandidate*>* collection;
    std::vector<const BPHMomentumSelect*> momSelector;
    std::vector<const BPHVertexSelect*>   vtxSelector;
    std::vector<const BPHFitSelect*>      fitSelector;
  };

  // return map of names to simple or previously recontructed particles
  // for currently tested combination 
  static std::map<std::string,const reco::Candidate *>& daugMap();
  static std::map<std::string,const BPHRecoCandidate*>& compMap();

  const edm::EventSetup* evSetup;
  double minPDiff;

  // list of simple and previously recontructed particles in the decay
  std::vector<BPHRecoSource*> sourceList;
  std::vector<BPHCompSource*> srCompList;

  // set of copies of previously reconstructed particles list
  // for bookkeeping and cleanup
  std::set<const std::vector<const BPHRecoCandidate*>*> compCollectList;

  // list fo selections to reconstructed particle
  std::vector<const BPHMomentumSelect*> msList;
  std::vector<const BPHVertexSelect  *> vsList;
  std::vector<const BPHFitSelect     *> fsList;

  // map linking particles names to position in list position
  std::map<std::string,int> sourceId;
  std::map<std::string,int> srCompId;

  // recursive function to build particles combinations
  void build( std::vector<ComponentSet>& compList,
              ComponentSet& compSet,
              std::vector<BPHRecoSource*>::const_iterator r_iter,
              std::vector<BPHRecoSource*>::const_iterator r_iend,
              std::vector<BPHCompSource*>::const_iterator c_iter,
              std::vector<BPHCompSource*>::const_iterator c_iend ) const;

  // check for already used particles in a combination
  // previously recontructed particles are assumed to be included
  // after simple particles
  bool contained( ComponentSet& compSet,
                  const reco::Candidate* cand ) const;
  bool contained( ComponentSet& compSet,
                  const BPHRecoCandidate* cand ) const;
  // compare two particles with their track reference and return 
  // true or false for same or different particles, including a
  // check with momentum difference
  bool sameTrack( const reco::Candidate* lCand,
                  const reco::Candidate* rCand ) const;

};


template <class T>
BPHRecoBuilder::BPHGenericCollection* BPHRecoBuilder::createCollection(
                                      const edm::Handle<T>& collection ) {
  return new BPHSpecificCollection<T>( *collection );
}


template <class T>
void BPHRecoBuilder::add( const std::string& name,
                          const edm::Handle<T>& collection,
                          double mass,
                          double msig ) {
  // forward call after creating an interface to the collection
  add( name, new BPHSpecificCollection<T>( *collection ), mass, msig );
  return;
}


template <class T>
void BPHRecoBuilder::add( const std::string& name,
                          const std::vector<const T*>& collection ) {
  // forward call after converting the list of pointer to a list
  // of pointer to base objects
  int i;
  int n = collection.size();
  std::vector<const BPHRecoCandidate*>* compCandList =
       new std::vector<const BPHRecoCandidate*>( n );
  for ( i = 0; i < n; ++i ) (*compCandList)[i] = collection[i];
  // save the converted list for cleanup
  compCollectList.insert( compCandList );
  add( name, *compCandList );
  return;
}


#endif // BPHRecoBuilder_H

