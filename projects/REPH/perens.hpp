#ifndef _PERENS_HPP
#define _PERENS_HPP

#include <tranalisi.hpp>

#include <REPH/base.hpp>

//! Incapsulate all info and operations
struct perens_t
{
  //! Folder where to find data and create plots
  const std::string dirPath;
  
  //! Spatial size
  int L;
  
  //! Temporal size
  int T;
  
  //! Total spatial volume
  int spatVol;
  
  //! Number of masses
  size_t nMass;
  
  //! List of mass
  vector<double> mass;
  
  //! Number of momenta
  size_t nMoms;
  
  //! List of momenta
  vector<array<double,3>> moms;
  
  //! Index spanning all momenta combination
  index_t indMesKin;
  
  //! Number of momenta combination
  size_t nMesKin;
  
  //! Index spanning all decay momenta combination
  index_t indDecKin;
  
  //! Returnt he tag of decKin ind
  string decKinTag(const size_t iMoms,const size_t iMomt,const size_t iMom0) const
  {
    return combine("iMomS%zu_iMomT%zu_iMomO%zu",iMoms,iMomt,iMom0);
  }
  
  //! Wrapper to the function returning the tag
  string decKinTag(const size_t iDecKin) const
  {
    const vector<size_t> c=indDecKin(iDecKin);
    
    return decKinTag(c[0],c[1],c[2]);
  }
  
  //! Number of decay momenta combination
  size_t nDecKin;
  
  //! Momenta of mesons in all kinematics
  vector<double> pMes;
  
  //! Maximal momentum
  double pMesMax;
  
  //! State whether or not to consider to study the decay
  vector<bool> considerDec;
  
  //! State whether there is a symmetric decay
  vector<bool> hasSymmDec;
  
  //! Symmetric of the decay
  vector<int> symmOfDec;
  
  //! Index of meson kinematic of the decay
  vector<int> iMesKinOfDecKin;
  
  //! Energy of the photon
  vector<double> Eg;
  
  //! Finite size correction for the photon energy
  double EgT(const size_t iDecKin) const
  {
    return sinh(Eg[iDecKin])*(1-exp(-T*Eg[iDecKin]));
  }
  
  //! Momentum of the decaying pair of lepton
  vector<double> kDec;
  
  //! Lattice version of the decaying momentum
  vector<double> kHatDec;
  
  //! Read the input file
  void readInput()
  {
    raw_file_t fin(combine("%s/jacks/input.txt",dirPath.c_str()),"r");
    
    L=fin.read<size_t>("L");
    spatVol=L*L*L;
    T=fin.read<size_t>("T");
    
    nMass=fin.read<size_t>("NMass");
    mass.resize(nMass);
    for(size_t iMass=0;iMass<nMass;iMass++)
      mass[iMass]=fin.read<double>();
    
    nMoms=fin.read<size_t>("NMoms");
    moms.resize(nMoms);
    for(size_t iMom=0;iMom<nMoms;iMom++)
      for(size_t mu=0;mu<3;mu++)
	moms[iMom][mu]=fin.read<double>();
  }
  
  //! Set all kinematics
  void setKinematics()
  {
    indMesKin.set_ranges({{"mom1",nMoms},{"mom2",nMoms}});
    nMesKin=indMesKin.max();
    
    indDecKin.set_ranges({{"iMoms",nMoms},{"iMomt",nMoms},{"iMom0",nMoms}});
    nDecKin=indDecKin.max();
    
    pMes.resize(nMesKin);
    
    pMesMax=0;
    for(size_t iKin=0;iKin<nMesKin;iKin++)
      {
	const vector<size_t> c=indMesKin(iKin);
	
	pMes[iKin]=2*M_PI*(moms[c[1]][2]-moms[c[0]][2])/L;
	
	pMesMax=std::max(pMesMax,fabs(pMes[iKin]));
      }
    
    resizeListOfContainers({&considerDec,&hasSymmDec},indDecKin.max());
    resizeListOfContainers({&symmOfDec,&iMesKinOfDecKin},indDecKin.max(),-1);
    resizeListOfContainers({&Eg,&kDec,&kHatDec},indDecKin.max());
    
    for(size_t iDecKin=0;iDecKin<indDecKin.max();iDecKin++)
      {
     	const vector<size_t> cDec=indDecKin(iDecKin);
	const size_t iMoms=cDec[0],iMomt=cDec[1],iMom0=cDec[2];
	considerDec[iDecKin]=(iMomt!=iMom0);
	hasSymmDec[iDecKin]=(iMoms==iMom0);
	if(hasSymmDec[iDecKin])
	  symmOfDec[iDecKin]=indDecKin({iMomt,iMoms,iMomt});
	
	kDec[iDecKin]=2*M_PI*(moms[iMom0][2]-moms[iMomt][2])/L;
	kHatDec[iDecKin]=2*sin(kDec[iDecKin]/2);
	
    	Eg[iDecKin]=2*asinh(fabs(kHatDec[iDecKin])/2);
	iMesKinOfDecKin[iDecKin]=indMesKin({iMoms,iMom0});
      }
  }
  
  //! Constructor
  perens_t(const std::string dirPath) : dirPath(dirPath)
  {
    readInput();
    
    setKinematics();
  }
};

#endif
