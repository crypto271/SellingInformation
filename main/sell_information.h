#ifndef _SELL_INFORMATION_
#define _SELL_INFORMATION_

#include "signature_key.h"

#include "../shared_signature/shared_sig.h"
#include "../common/common.h"
#include "../common/aes.h"
#include "../protocol.h"
#include "../common/square_root.h"

#include <cryptopp/integer.h>
#include <cryptopp/osrng.h>

#include <atomic>

namespace sell_information {

class Seller {
	// p*q = n, Buyer wants to buy p, q
	CryptoPP::Integer p;	
	CryptoPP::Integer q;
	CryptoPP::Integer n;

	unsigned price;
  bool cheat;

	common::SquareRoot square_root;

	std::array<CryptoPP::Integer, common::L> y;
	std::array<CryptoPP::Integer, common::L> r1;
	std::array<CryptoPP::Integer, common::L> r2;

	std::array<std::vector<byte>, common::L> c1;
	std::array<std::vector<byte>, common::L> c2;

	Transaction T1;
	public:

	SingleSeller single_seller;

	std::array<sha_commitment::Sender, common::L> d1;
	std::array<sha_commitment::Sender, common::L> d2;

	Seller(CryptoPP::Integer p, 
				CryptoPP::Integer q, 
				unsigned price,
        bool cheat):
		p(p), q(q), n(p*q), price(price), cheat(cheat), square_root(p, q) {}

	CryptoPP::Integer get_n(){
		return p*q;
	}

	unsigned get_price(){
		return price;
	}
	
	void acceptSquares(const std::array<CryptoPP::Integer, common::L>& y){
		this->y = y;
	}

	void findRoots();
	
	void encryptRoots();

	void setSingleSeller(const SingleSeller& single_seller){
		this->single_seller = single_seller;
	}	

	void setupCommit();

	std::array<unsigned, common::L/2> acceptSubset(
		const std::array<unsigned, common::L/2>& indices, 
		const std::array<CryptoPP::Integer, common::L/2>& values);

	void accept_T1(const Transaction& T1);
	void accept_payment();
};

class Buyer {
	CryptoPP::Integer n;	// The rsa modulus - would like to buy primes p, q: p*q = n

	unsigned price;

	std::array<CryptoPP::Integer, common::L> x;
	std::array<CryptoPP::Integer, common::L> y;

	int r;

	std::vector<unsigned> indices;

	std::vector<byte> signature;

	public:
	// results
	CryptoPP::Integer p;
	CryptoPP::Integer q;

	SingleBuyer single_buyer;

	std::array<sha_commitment::Receiver, common::L> d1;
	std::array<sha_commitment::Receiver, common::L> d2;

	Buyer(CryptoPP::Integer n, 
				unsigned price):
		n(n), price(price) {}

	CryptoPP::Integer get_n(){
		return n;
	}

	unsigned get_price(){
		return price;
	}

	std::vector<byte> getSignature() {
		return signature;
	}

	void setSignature(std::vector<byte> signature) {
		this->signature = signature;
	}

	void pickR();

	int getR(){
		return r;
	}

	void genSquares(){
		for(unsigned i = 0; i < common::L; ++i) {
			x[i] = CryptoPP::Integer(common::rng(), 0, n/2);
			y[i] = (x[i] * x[i]) % n;
		}
	}

	std::array<CryptoPP::Integer, common::L> getSquares(){
		return y;
	}

	void setSingleBuyer(const SingleBuyer& single_buyer) {
		this->single_buyer = single_buyer;
	}

	void pickSubset();
	std::array<unsigned, common::L/2> getSubsetIndices();
	std::array<CryptoPP::Integer, common::L/2> getSubsetValues();

	void verifyRoot(unsigned ind, const std::vector<byte>& key, const std::vector<byte>& c);

	void factorise();
	void make_payment();
	void wait_for_signature_or_time_lock();
	void solve_time_lock(std::atomic_bool &finished);
	void get_signature(std::atomic_bool &finished);

	void set_signature(const std::vector<byte>& signature) {  // for testing
		this->signature = signature;	
	}
};

class SellInformationProtocol : public Protocol<Seller, Buyer> {
	public:
	void init(Seller *, Buyer *);
	void exec(Seller *, Buyer *);
	void open(Seller *, Buyer *);
};

}

#endif

