/**
 * @file   ConditionalGaussian.cpp
 * @brief  Conditional Gaussian Base class
 * @author Christian Potthast
 */

#include <string.h>
#include <boost/numeric/ublas/vector.hpp>
#include "Ordering.h"
#include "ConditionalGaussian.h"

using namespace std;
using namespace gtsam;

/* ************************************************************************* */
ConditionalGaussian::ConditionalGaussian(const string& key,Vector d, Matrix R, Vector precisions) :
	Conditional (key), R_(R),precisions_(precisions),d_(d)
{
}

/* ************************************************************************* */
ConditionalGaussian::ConditionalGaussian(const string& key, Vector d, Matrix R,
		const string& name1, Matrix S, Vector precisions) :
	Conditional (key), R_(R), precisions_(precisions), d_(d) {
	parents_.insert(make_pair(name1, S));
}

/* ************************************************************************* */
ConditionalGaussian::ConditionalGaussian(const string& key, Vector d, Matrix R,
		const string& name1, Matrix S, const string& name2, Matrix T, Vector precisions) :
	Conditional (key), R_(R),precisions_(precisions), d_(d) {
	parents_.insert(make_pair(name1, S));
	parents_.insert(make_pair(name2, T));
}

/* ************************************************************************* */
ConditionalGaussian::ConditionalGaussian(const string& key,
		const Vector& d, const Matrix& R, const map<string, Matrix>& parents, Vector precisions) :
	Conditional (key), R_(R),precisions_(precisions), d_(d), parents_(parents) {
}

/* ************************************************************************* */
void ConditionalGaussian::print(const string &s) const
{
  cout << s << ":" << endl;
  gtsam::print(R_,"R");
  for(Parents::const_iterator it = parents_.begin() ; it != parents_.end() ; it++ ) {
    const string&   j = it->first;
    const Matrix& Aj = it->second;
    gtsam::print(Aj, "A["+j+"]");
  }
  gtsam::print(d_,"d");
  gtsam::print(precisions_,"precisions");
}    

/* ************************************************************************* */
bool ConditionalGaussian::equals(const Conditional &c, double tol) const {
	if (!Conditional::equals(c)) return false;
	const ConditionalGaussian* p = dynamic_cast<const ConditionalGaussian*> (&c);
	if (p == NULL) return false;
	Parents::const_iterator it = parents_.begin();

	// check if the size of the parents_ map is the same
	if (parents_.size() != p->parents_.size()) return false;

	// check if R_ is equal
	if (!(equal_with_abs_tol(R_, p->R_, tol))) return false;

	// check if d_ is equal
	if (!(::equal_with_abs_tol(d_, p->d_, tol))) return false;

	// check if precisions are equal
	if (!(::equal_with_abs_tol(precisions_, p->precisions_, tol))) return false;

	// check if the matrices are the same
	// iterate over the parents_ map
	for (it = parents_.begin(); it != parents_.end(); it++) {
		Parents::const_iterator it2 = p->parents_.find(it->first.c_str());
		if (it2 != p->parents_.end()) {
			if (!(equal_with_abs_tol(it->second, it2->second, tol))) return false;
		} else
			return false;
	}
	return true;
}

/* ************************************************************************* */
list<string> ConditionalGaussian::parents() const {
	list<string> result;
	for (Parents::const_iterator it = parents_.begin(); it != parents_.end(); it++)
		result.push_back(it->first);
	return result;
}

/* ************************************************************************* */
Vector ConditionalGaussian::solve(const VectorConfig& x) const {
	Vector rhs = d_;
	for (Parents::const_iterator it = parents_.begin(); it
			!= parents_.end(); it++) {
		const string& j = it->first;
		const Matrix& Aj = it->second;
		rhs -= Aj * x[j];
	}
	Vector result = backsubstitution(R_, rhs);
	return result;
}

/* ************************************************************************* */
