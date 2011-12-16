/**
 * @file radical_main.cpp
 * @author Nishant Mehta
 *
 * Executable for RADICAL
 */
#include <armadillo>
#include <mlpack/core.hpp>
#include <mlpack/methods/radical/radical.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(RadicalTest);

using namespace mlpack;
using namespace mlpack::radical;
using namespace std;
using namespace arma;

BOOST_AUTO_TEST_CASE(Radical_Test_Radical3D) {
  mat matX;
  data::Load("data_3d_mixed.txt", matX);

  radical::Radical rad(0.175, 5, 100, matX.n_rows - 1);
  
  mat matY;
  mat matW;  
  rad.DoRadical(matX, matY, matW);  
  
  mat matYT = trans(matY);
  double valEst = 0;
  
  for(u32 i = 0; i < matYT.n_cols; i++) 
  {
    vec y = vec(matYT.col(i));
    valEst += rad.Vasicek(y);
  }  
  
  mat matS;
  data::Load("data_3d_ind.txt", matS);  
  rad.DoRadical(matS, matY, matW);
    
  matYT = trans(matY);
  double valBest = 0;
  
  for (u32 i = 0; i < matYT.n_cols; i++) 
  {
    vec y = vec(matYT.col(i));
    valBest += rad.Vasicek(y);
  }
  
  /*
  printf("valBest = %f\n", valBest);
  printf("valEst = %f\n", valEst);
  printf("\n\nabs(valBest - valEst) = %f\n", fabs(valBest - valEst));
  printf("\n\n\t\t%f\n", fabs(valBest - valEst) / fabs(valBest));
  printf("\n\n\t\t%f\n", fabs(valBest - valEst) / fabs(valEst));
  */
    
  BOOST_REQUIRE_CLOSE(valBest, valEst, 0.2);
  
}

BOOST_AUTO_TEST_SUITE_END();
