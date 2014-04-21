#include <iostream>
#include <random>
#include <algorithm>
#include <cmath>

#include "BasicNetwork.h"

BasicNetwork::BasicNetwork(const std::vector<size_t>& sizes) : 
    sizes_(sizes),
    num_layers_(sizes.size())
{
    weights_.reserve(num_layers_ - 1);
    biases_.reserve(num_layers_ - 1);

    for (int i = 0; i < sizes_.size() - 1; ++i) {
        const size_t x = sizes_[i];
        const size_t y = sizes_[i + 1];
        
        // Initialize the weights and biases
        // to normally distributed values
        auto rand_normal = [](double val){
            static std::random_device rd;
            static std::mt19937_64 gen(rd());
            static std::normal_distribution<> d(0, 1);

            return d(gen);
        };
        MatrixXd layer_weights = MatrixXd::Zero(x, y).unaryExpr(rand_normal);
        MatrixXd layer_biases = MatrixXd::Zero(x, 1).unaryExpr(rand_normal);

        weights_.push_back(layer_weights);
        biases_.push_back(layer_biases);
    }
}

void BasicNetwork::TrainSGD(std::vector<DataPair> training_data,
    const size_t epochs,
    const size_t mini_batch_size,
    const float  eta,
    const  std::vector<DataPair>& test_data = {}) {

    int n_data = training_data.size();

    for (int i = 0; i < epochs; ++i) {
        std::random_shuffle(training_data.begin(), training_data.end());
        std::vector<std::vector<DataPair>> mini_batches;

        // Create the mini batches
        for (int j = 0; j < n_data; j += mini_batch_size) {
            std::vector<DataPair> batch;
            for (int k = i; k < i + mini_batch_size; ++k) {
                batch.push_back(training_data[k]);
            }
            mini_batches.push_back(std::move(batch));
        }

        for (auto& mini_batch : mini_batches) {
            update_mini_batch(mini_batch, eta);
        }
        std::cout << "Epoch " << i + 1 << std::endl;
    }
}

void BasicNetwork::update_mini_batch(std::vector<DataPair>& mini_batch,
    const float eta) {

    // Initialize to matrices with 0s
    MatrixList nabla_w(weights_);
    MatrixList nabla_b(biases_);
    std::for_each(nabla_w.begin(), nabla_w.end(), [](MatrixXd& mat){mat.setZero();});
    std::for_each(nabla_b.begin(), nabla_b.end(), [](MatrixXd& mat){mat.setZero();});

    // Compute the gradient by applying the back-propagation
    // algorithm to each example in the mini batch and
    // summing the resulting lists of matrices elementwise.
    for (auto& example : mini_batch) {
        auto delta_nabla = back_propagation(example);

        auto delta_nabla_w = delta_nabla.first;
        auto delta_nabla_b = delta_nabla.second;

        for (int i = 0; i < num_layers_; ++i) {
            nabla_w[i] += delta_nabla_w[i];
            nabla_b[i] += delta_nabla_b[i];
        }
    }

    // Update the weights and biases based on the learning rate
    for (int i = 0; i < num_layers_; ++i) {
        weights_[i] -= eta * nabla_w[i];
        biases_[i]  -= eta * nabla_b[i];
    }
}

std::pair<MatrixList, MatrixList> BasicNetwork::back_propagation(const DataPair& training_example) {
    return std::make_pair(weights_, biases_);
}

VectorXd BasicNetwork::cost_derivative(const VectorXd& output_activations,
    const VectorXd& y) {

    return output_activations - y;
}

VectorXd BasicNetwork::feed_forward(VectorXd a) {
    for (int i = 0; i < weights_.size(); i++) {
        a = sigmoid_vec(weights_[i] * a + biases_[i]);
    }
    return a;
}

void BasicNetwork::evaluate(std::vector<DataPair>& test_data) {
}

double BasicNetwork::sigmoid_func(double z) {
    return 1/(1 + std::exp(-z));
}

VectorXd BasicNetwork::sigmoid_vec(const VectorXd& z) {
    return z.unaryExpr([](double arg) { 
        return BasicNetwork::sigmoid_func(arg);
    });
}

VectorXd BasicNetwork::sigmoid_prime_vec(const VectorXd& z) {
    return z.unaryExpr([](double arg) { 
        double sigz = BasicNetwork::sigmoid_func(arg);
        return -sigz/(1 - sigz);
    });
}
