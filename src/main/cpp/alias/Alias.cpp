/*
 * Alias.cpp
 *
 *  Created on: Mar 11, 2017
 *      Author: cedric
 */

#include "Alias.h"
#include "AliasUtil.h"

#include <exception>
#include <math.h>
#include <assert.h>
#include <random>

namespace stride{
namespace alias{

Alias::Alias(std::vector<double> probabilities) : Alias(probabilities,std::random_device()()) {}

Alias::Alias(std::vector<double> probabilities,unsigned int seed){
	assert(probabilities.size()>0);
	if(probabilities.size()<=0){
		throw EmptyProbabilityException();
	}
	m_random = util::Random(seed);
	unsigned int n = probabilities.size();
	m_prob.resize(n);
	m_alias.resize(n);
	std::vector<unsigned int> small, large;
	for(std::vector<double>::iterator i = probabilities.begin(); i< probabilities.end(); i++){
		*i *= double(n);
	}
	for(unsigned int i = 0; i< probabilities.size(); i++){
		if(probabilities[i] < 1.0){
			small.push_back(i);
		}
		else{
			large.push_back(i);
		}
	}

	while(!(small.empty()||large.empty())){
		unsigned int l = large.front();
		large.erase(large.begin());
		unsigned int g = small.front();
		small.erase(small.begin());
		m_prob[l] = probabilities[l];
		m_alias[l] = g;

		probabilities[g] = probabilities[g]+probabilities[l]-1;
		if(probabilities[g] >= 1){
			large.push_back(g);
		}
		else{
			small.push_back(g);
		}
	}

	while(!large.empty()){
		unsigned int g = large.front();
		large.erase(large.begin());
		m_prob[g] = 1;
	}

	while(!small.empty()){
		unsigned int l = small.front();
		small.erase(small.begin());
		m_prob[l] = 1;
	}
}

unsigned int Alias::Next(){
	unsigned int roll = m_random(m_alias.size());
	double flip = m_random.NextDouble();
	if(flip <= m_prob[roll]){
		return roll;
	}
	else{
		return m_alias[roll];
	}


}
}
}
