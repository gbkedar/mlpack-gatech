/**
 * @file scf_solver_test.cc
 * 
 * @author Bill March (march@gatech.edu)
 *
 * Unit tests for the Hartree-Fock code.  
 */

#include "fastlib/base/test.h"
#include "scf_solver.h"


/**
 * Unit test class for Hartree-Fock code.
 */
class SCFSolverTest {
  
  // Use the example from Leach's book
  
public:
  
  
  static const size_t num_electrons = 2;
  static const double eps = 0.01;
  
  void Init() {
    
    solver_ = new HFSolver();
  
    Matrix overlap;
    Matrix kinetic;
    Matrix potential;
    ArrayList<double> two_electron;
    
    Matrix core;
    
    double nuclear_energy = 0.0;
    
    data::Load("overlap_test.csv", &overlap);
    data::Load("kinetic_test.csv", &kinetic);
    data::Load("potential_test.csv", &potential);
    //data::Load("two_electron_test.csv", &two_electron);
    two_electron.Init();
    
    data::Load("core_test.csv", &core);
    
    solver_->Init(nuclear_energy, overlap, kinetic, potential, two_electron, 
                 num_electrons);
    
    for (size_t i = 0; i < core.n_rows(); i++) {
      for (size_t j = 0; j < core.n_cols(); j++) {
        TEST_DOUBLE_APPROX(core.ref(i, j), 
                           solver_->one_electron_integrals_.ref(i, j), eps);
      }
    }
    
    
  }
  
  void Destruct() {
   
    delete solver_;
    
  }
  
  void TestOrthogonalizingMatrix() {
    Init();
    
    solver_->FormOrthogonalizingMatrix_();
    
    Matrix true_orthogonal;
    data::Load("orthogonalizing_test.csv", &true_orthogonal);
    
    // Is the change-of-basis matrix unique with respect to negaives on the 
    // diagonal?  If not, I need to add absolute values here.
    for (size_t i = 0; i < true_orthogonal.n_rows(); i++) {
      for (size_t j = 0; j < true_orthogonal.n_cols(); j++) {
        TEST_DOUBLE_APPROX(true_orthogonal.ref(i, j), 
                           solver_->overlap_matrix_.ref(i, j), eps); 
      }
    }
    
    Destruct();
    
    NONFATAL("Orthogonal matrix correct.\n");
  }
  
  void TestDensityMatrix() {
    
    Init();
    
    solver_->FormOrthogonalizingMatrix_();
    
    solver_->ComputeDensityMatrix_();
    
    // Check to see that it worked for real
    Matrix true_density;
    data::Load("density_test.csv", &true_density);
    
    for (size_t i = 0; i < true_density.n_rows(); i++) {
      for (size_t j = 0; j < true_density.n_cols(); j++) {
        TEST_DOUBLE_APPROX(true_density.ref(i, j), 
                           solver_->density_matrix_.ref(i, j), eps);
      }
    }
    
    NONFATAL("Initial Density Matrix Correct.\n");
    
    Destruct();
    
  } // TestDensityMatrix
  
  void TestFillOrbitals() {
    
    Init();
    
    solver_->number_of_electrons_ = 5;
    solver_->number_of_basis_functions_ = 10;
    
    Vector test_energy_vector;
    test_energy_vector.Init(10);
    test_energy_vector[0] = -5;
    test_energy_vector[1] = 12;
    test_energy_vector[2] = 1;
    test_energy_vector[3] = 7;
    test_energy_vector[4] = 3;
    test_energy_vector[5] = -9;
    test_energy_vector[6] = 4;
    test_energy_vector[7] = 1;
    test_energy_vector[8] = 0;
    test_energy_vector[9] = 2;
    
    solver_->energy_vector_ = test_energy_vector;
    
    ArrayList<size_t> test_indices;
    solver_->FillOrbitals_(&test_indices);
    
    TEST_ASSERT((test_indices[0] == 0) && (test_indices[1] == 5) 
                && (test_indices[2] == 8));
    
    
    NONFATAL("FillOrbitals_ correct\n");

    
    Destruct();
    
  } // TestFillOrbitals
  
  void TestFindIntegralIndex() {
    
    size_t test1 = solver_->FindIntegralIndexHelper_(1, 0);
    
    size_t test2 = solver_->FindIntegralIndexHelper_(5, 4);
    
    size_t test3 = solver_->FindIntegralIndex_(0, 0, 0, 0);
    
    size_t test4 = solver_->FindIntegralIndex_(1, 1, 0, 0);
    
    size_t test5 = solver_->FindIntegralIndex_(1, 0, 1, 0);
    
    TEST_ASSERT(test1 == 1);
    TEST_ASSERT(test2 == 19);
    TEST_ASSERT(test3 == 0);
    TEST_ASSERT(test4 == 3);
    TEST_ASSERT(test5 == 2);
    
    NONFATAL("FindIntegralIndex correct.\n");

  } // TestFindIntegralIndex
  
  void TestAll() {
   
    TestOrthogonalizingMatrix();  
    
    TestDensityMatrix();
    
    TestFillOrbitals();
    
    TestFindIntegralIndex();
    
    NONFATAL("All tests passed\n");
    
  }
  
private:
  
  SCFSolver* solver_;
  
}; // class SCFSolverTest

// There's some kind of weird bug in the apple loader.  eps won't be recognized
// properly without this line.  
const double HartreeFockTest::eps;


int main(int argc, char *argv[]) {

  HartreeFockTest tester;
  tester.TestAll();
  
  return 0;
  
}