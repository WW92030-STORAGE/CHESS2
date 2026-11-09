#include <bits/stdc++.h>
using namespace std;
#include "Chess2.h"
#include "Chess2GameUtils.h"
#include "Chess2Search.h"
#include "blackboxies.h"
#include "Chess2Training.h"

int main() {
	int DEPTH = 2;
	auto nnue = breedNNUE(16, 8, 4, DEPTH);
	ofstream fout("nnue.txt");
	fout << "int DEPTH = " << DEPTH << "\n";
	fout << nnue[0].formatHiddenWeights() << "\n" << nnue[0].formatOutputWeights() << "\n";
	fout << nnue[1].formatHiddenWeights() << "\n" << nnue[1].formatOutputWeights() << "\n";

	fout.close();

	std::cout << "File closed\n";

	return 0;
}
