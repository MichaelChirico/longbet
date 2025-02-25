#ifndef GUARD_tree_h
#define GUARD_tree_h

#include <map>
#include <cmath>
#include <cstddef>
#include <memory>
#include <vector>
#include "common.h"
#include "sample_int_crank.h"
#include "model.h"
#include "X_struct.h"
#include "split_info.h"
#include "json.h"

// local modification msquinn@: Use json in third_party/
// #include "third_party/json/src/json.hpp"

//#include <armadillo>

// for convenience
using json = nlohmann::json;

void calcSuffStat_categorical(std::vector<double> &temp_suff_stat, std::vector<size_t> &xorder, std::vector<size_t> &torder, size_t &start, size_t &end, Model *model, std::unique_ptr<State> &state);

void calcSuffStat_continuous(std::vector<double> &temp_suff_stat, std::vector<size_t> &xorder, std::vector<size_t> &torder, std::vector<size_t> &candidate_index, size_t index, bool adaptive_cutpoint, Model *model, std::unique_ptr<State> &state);

void calcSuffStat_time(std::vector<double> &temp_suff_stat,
std::vector<size_t> &xorder, std::vector<size_t> &torder, size_t &start,
size_t &end, Model *model, std::unique_ptr<State> &state);

// void calc_suff_continuous(std::vector<size_t> &xorder, std::vector<double> &y_std, std::vector<size_t> &candidate_index, size_t index, double &suff_stat, bool adaptive_cutpoint);

//--------------------------------------------------
//BART likelihood function
//--------------------------------------------------
//generate a vector of integers
// arma::uvec range(size_t start, size_t end); Removed

//--------------------------------------------------
//info contained in a node, used by input operator
struct node_info
{
    std::size_t id; //node id
    std::size_t v;  //variable
    std::size_t c_index; // index of cutpoint in the design matrix, leave it blank, just for XBCF
    double c;       //cut point // different from BART
    std::vector<double> theta_vector;
};

//--------------------------------------------------
class tree
{
public:
    // std::vector<double> theta_vector;
    std::vector<double> theta_vector;

    std::vector<double> suff_stat;

    //typedefs--------------------
    typedef tree *tree_p;

    typedef const tree *tree_cp;

    typedef std::vector<tree_p> npv;

    typedef std::vector<tree_cp> cnpv;

    //contructors,destructors--------------------
    tree() : depth(0), suff_stat(3, 0.0), theta_vector(1, 0.0), v(0), c_index(0), c(0), split_t(false), no_split(false), p(0), l(0), r(0), prob_split(0.0), prob_leaf(0.0), drawn_ind(0), loglike_node(0.0), tree_like(0.0), num_cutpoint_candidates(0), ID(1) {}

    tree(size_t dim_suffstat) : depth(0), suff_stat(dim_suffstat, 0.0), theta_vector(1, 0.0), v(0), c_index(0), c(0), split_t(false), no_split(false), p(0), l(0), r(0), prob_split(0.0), prob_leaf(0.0), drawn_ind(0), loglike_node(0.0), tree_like(0.0), num_cutpoint_candidates(0), ID(1) {}

    tree(const tree &n) : depth(0), suff_stat(2, 0.0), theta_vector(1, 0.0), v(0), c_index(0), c(0), split_t(false), no_split(false), p(0), l(0), r(0), prob_split(0.0), prob_leaf(0.0), drawn_ind(0), loglike_node(0.0), tree_like(0.0), num_cutpoint_candidates(0) { cp(this, &n); }

    tree(double itheta) : depth(0), suff_stat(2, 0.0), theta_vector(itheta, 0.0), v(0), c_index(0), c(0), split_t(false), no_split(false), p(0), l(0), r(0), prob_split(0.0), prob_leaf(0.0), drawn_ind(0), loglike_node(0.0), tree_like(0.0), num_cutpoint_candidates(0) {}

    tree(size_t dim_theta, const tree_p parent, size_t dim_suffstat) : suff_stat(dim_suffstat, 0.0), theta_vector(dim_theta, 0.0), v(0), c_index(0), c(0), split_t(false), no_split(false), p(parent), l(0), r(0), prob_split(0.0), prob_leaf(0.0), drawn_ind(0), loglike_node(0.0), tree_like(0.0), num_cutpoint_candidates(0) {}

    tree(size_t dim_theta, size_t dim_suffstat) : suff_stat(dim_suffstat, 0.0), theta_vector(dim_theta, 0.0), v(0), c_index(0), c(0), split_t(false), no_split(false), p(0), l(0), r(0), prob_split(0.0), prob_leaf(0.0), drawn_ind(0), loglike_node(0.0), tree_like(0.0), num_cutpoint_candidates(0) {}
    
    void tonull(); //like a "clear", null tree has just one node

    ~tree() { tonull(); }

    //operators----------
    tree &operator=(const tree &);

    //interface--------------------
    //set
    void settheta(std::vector<double> theta_vector) { this->theta_vector = theta_vector; }

    void setv(size_t v) { this->v = v; }

    void setc(size_t c) { this->c = c; }

    void setc_index(size_t c_index) { this->c_index = c_index; }

    //get
    std::vector<double> gettheta_vector() const { return theta_vector; }

    double getprob_split() const { return prob_split; }

    double getprob_leaf() const { return prob_leaf; }

    size_t getv() const { return v; }

    double getc() const { return c; }

    size_t getc_index() const { return c_index; }

    double getloglike_node() const { return loglike_node; }

    size_t getdepth() const { return depth; }

    double gettree_like() const { return tree_like; }

    size_t getnum_cutpoint_candidates() const { return num_cutpoint_candidates; }

    void setnum_cutpoint_candidates(size_t x) { this->num_cutpoint_candidates = x; }

    tree_p getp() { return p; }

    tree_p getl() { return l; }

    tree_p getr() { return r; }

    size_t getID() { return ID; }

    void setID(size_t ID) { this->ID = ID; }

    size_t getN() { return N; }

    void setN(size_t N) { this->N = N; }

    //tree functions--------------------
    tree_p getptr(size_t nid); //get node pointer from node id, 0 if not there

    void pr(bool pc = true); //to screen, pc is "print children"

    size_t treesize(); //number of nodes in tree

    size_t nnogs(); //number of nog nodes (no grandchildren nodes)

    size_t nbots(); //number of bottom nodes

    void getbots(npv &bv); //get bottom nodes

    void getnogs(npv &nv); //get nog nodes (no granchildren)

    void getnodes(npv &v); //get vector of all nodes

    void getnodes(cnpv &v) const; //get vector of all nodes (const)

    tree_p gettop(); // get pointer to the top node

    void ini_suff_stat() { std::fill(suff_stat.begin(), suff_stat.end(), 0.0); }

    void resize_suff_stat(size_t dim_suffstat)
    {
        suff_stat.resize(dim_suffstat);
        std::fill(suff_stat.begin(), suff_stat.end(), 0.0);
    };

    void grow_from_root(std::unique_ptr<State> &state, std::unique_ptr<split_info> &split, Model *model, std::unique_ptr<X_struct> &x_struct, const size_t &sweeps, const size_t &tree_ind, bool control_split_t);
    
    tree_p bn(double *x, matrix<double> &xi); //find Bottom Node, original BART version

    tree_p bn_std(double *x); // find Bottom Node, std version, compare

    tree_p search_bottom_std(const double *X, const double *t, const size_t &i, const size_t &j, const size_t &N, const size_t &N_t);

    void rg(size_t v, size_t *L, size_t *U); //recursively find region [L,U] for var v
    //node functions--------------------

    size_t nid() const; //nid of a node
    // size_t depth();     //depth of a node

    char ntype(); //node type t:top, b:bot, n:no grandchildren i:interior (t can be b)

    bool isnog();

    json to_json();

    void from_json(json &j3, size_t dim_theta);

    void cp(tree_p n, tree_cp o); //copy tree

    void copy_only_root(tree_p o); // copy tree, point new root to old structure

    //friends--------------------
    friend std::istream &operator>>(std::istream &, tree &);

    friend void BART_likelihood_all(std::unique_ptr<split_info> &split_info,
    bool &no_split, size_t &split_var, size_t &split_point,
    const std::vector<size_t> &subset_vars, Model *model,
    std::unique_ptr<X_struct> &x_struct, std::unique_ptr<State> &state,
    tree *tree_pointer, bool &split_t, bool control_split_t);

    friend void calculate_loglikelihood_continuous(std::vector<double> &loglike, const std::vector<size_t> &subset_vars, size_t &N_Xorder, matrix<size_t> &Xorder_std, matrix<size_t> &torder_std, double &loglike_max, Model *model, std::unique_ptr<X_struct> &x_struct, std::unique_ptr<State> &state, tree *tree_pointer);

    friend void calculate_loglikelihood_categorical(std::vector<double> &loglike, size_t &loglike_start, const std::vector<size_t> &subset_vars, size_t &N_Xorder, matrix<size_t> &Xorder_std, matrix<size_t> &torder_std, double &loglike_max, std::vector<size_t> &X_counts, std::vector<size_t> &X_num_unique, Model *model, std::unique_ptr<X_struct> &x_struct, size_t &total_categorical_split_candidates, std::unique_ptr<State> &state, tree *tree_pointer);

    friend void calculate_likelihood_no_split(std::vector<double> &loglike, size_t &N_Xorder, double &loglike_max, Model *model, std::unique_ptr<X_struct> &x_struct, size_t &total_categorical_split_candidates, std::unique_ptr<State> &state, tree *tree_pointer);

    friend void calculate_loglikelihood_time(std::vector<double> &loglike,
    size_t &loglike_start, double &loglike_max, Model *model,
    std::unique_ptr<X_struct> &x_struct, std::unique_ptr<split_info> &split_info,
    std::unique_ptr<State> &state, tree *tree_pointer);

    // friend void split_xorder_std_continuous(matrix<size_t> &Xorder_left_std, matrix<size_t> &Xorder_right_std, size_t split_var, size_t split_point, matrix<size_t> &Xorder_std, matrix<size_t> &torder_std, Model *model, std::unique_ptr<X_struct> &x_struct, std::unique_ptr<State> &state, tree *current_node);

    // friend void split_xorder_std_categorical(matrix<size_t> &Xorder_left_std, matrix<size_t> &Xorder_right_std, size_t split_var, size_t split_point, matrix<size_t> &Xorder_std, matrix<size_t> &torder_std, std::vector<size_t> &X_counts_left, std::vector<size_t> &X_counts_right, std::vector<size_t> &X_num_unique_left, std::vector<size_t> &X_num_unique_right, std::vector<size_t> &X_counts, Model *model, std::unique_ptr<X_struct> &x_struct, std::unique_ptr<State> &state, tree *current_node);

    // friend void split_torder_std(matrix<size_t> &Xorder_std,
    // matrix<size_t> &torder_std, matrix<size_t> &torder_left_std,
    // matrix<size_t> &torder_right_std, size_t split_var, size_t split_point,
    // Model *model, std::unique_ptr<X_struct> &x_struct,
    // std::unique_ptr<State> &state, tree *current_node);

    // #ifndef NoRcpp
    // #endif
private:
    size_t N;  // number of data points in the level

    size_t ID;

    size_t depth;

    // rule: left if x[v] < matrix<double>[v][c]
    size_t v;  // index of variable to split

    size_t c_index;

    double c;

    bool split_t;  // indicator to split on time

    bool no_split;  // inidicator to split on covariates

    double prob_split;  // posterior of the chose split points, by Bayes rule

    double prob_leaf;  // posterior of the leaf parameter, mu

    double loglike_node;  // loglikelihood of the leaf data

    double tree_like;  // for debug use, likelihood of the tree

    size_t drawn_ind;  // index drawn when sampling cutpoints (in the total likelihood + nosplit vector)

    size_t num_cutpoint_candidates;  // number of cutpoint candidates

    // tree structure
    tree_p p;  // parent

    tree_p l;  // left child

    tree_p r;  // right child
};

std::istream &operator>>(std::istream &, tree &);
std::ostream &operator<<(std::ostream &, const tree &);

// void predict_from_tree(tree &tree, const double *X_std, size_t N, size_t p, std::vector<double> &output, Model *model);

// void predict_from_datapointers(size_t tree_ind, Model *model, std::unique_ptr<State> &state, std::unique_ptr<X_struct> &x_struct);

void getThetaForObs_Outsample(std::vector<double> &output, tree &tree, size_t x_index, size_t t_index, const double *Xtest, const double *tpointer, size_t N_Xtest, size_t p);

void getThetaForObs_Outsample_ave(matrix<double> &output, std::vector<tree> &tree, size_t x_index, const double *Xtest, size_t N_Xtest, size_t p);

#endif