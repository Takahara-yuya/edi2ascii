//--------------------------------------------------------------------------
// Copyright(c) 2025, Zuwei Huang
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
//
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and /or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//--------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <iomanip>
#include <complex>

const double PI = 3.14159265358979323846;
const double MU0 = 4.0 * PI * 1e-7;
const double RAD2DEG = 180.0 / PI;
using namespace std;
int main()
{
	double SI_TRANS = 4.0 * PI * 1e-4; // 
	//double SI_TRANS = 1; 

	string infile = "merge.edi";   // 
	cerr << "Input EDI file name: \n";
	cin >> infile;
	size_t pos = infile.find(".edi");
	std::string outfile = infile.substr(0, pos) + "_read.txt";
	std::string outfile1 = infile.substr(0, pos) + "_read_appphs.txt";

	ifstream fin(infile);
	if (!fin.is_open()) {
		cerr << "Cannot open input file\n";
		return 1;
	}

	map<string, vector<double>> data;
	string line, key;
	int nread = 0;

	while (getline(fin, line)) {
		if (line.empty()) continue;

		// 
		if (line[0] == '>') {
			stringstream ss(line);
			ss.ignore(1);          // jump '>'
			ss >> key;             // FREQ / ZXXR / ZXYI ...
			//
			if (key == "COH") {
				// which COH
				static int coh_count = 0;
				coh_count++;
				if (coh_count == 1) {
					key = "COH1";
				}
				else if (coh_count == 2) {
					key = "COH2";
				}
				//... can be extended if more COH present
			}

			// 
			size_t pos = key.find_first_of(" \t");
			if (pos != string::npos) {
				key = key.substr(0, pos);
			}

			size_t pos2 = line.find("//");
			if (pos2 == string::npos) continue;

			int n;
			n = stoi(line.substr(pos2 + 2));

			vector<double> values;
			values.reserve(n);

			while ((int)values.size() < n && getline(fin, line)) {
				stringstream vs(line);
				double v;
				while (vs >> v) {
					values.push_back(v);
				}
			}

			data[key] = values;
		}
	}
	fin.close();

	//
	if (data.find("FREQ") == data.end()) {
		cerr << "No FREQ data found\n";
		return 1;
	}

	size_t N = data["FREQ"].size();

	ofstream fout(outfile);
	ofstream fout1(outfile1);
	fout << scientific << setprecision(10);
	fout1 << scientific << setprecision(10);

	fout << "#FREQ\tZROT\tZXXR\tZXXI\tZXX.VAR\tZXYR\tZXYI\tZXY.VAR\t"
		<< "ZYXR\tZYXI\tZYX.VAR\tZYYR\tZYYI\tZYY.VAR\t"
		<< "TROT.EXP\tTXR.EXP\tTXI.EXP\tTXVAR.EXP\t"
		<< "TYR.EXP\tTYI.EXP\tTYVAR.EXP\tCOH1\tCOH2\n";

	fout1 << "#FREQ\tAPP_XX\tPHASE_XX\tAPP_XX_ERR\tPHASE_XX_ERR\t"
		<< "APP_XY\tPHASE_XY\tAPP_XY_ERR\tPHASE_XY_ERR\t"
		<< "APP_YX\tPHASE_YX\tAPP_YX_ERR\tPHASE_YX_ERR\t"
		<< "APP_YY\tPHASE_YY\tAPP_YY_ERR\tPHASE_YY_ERR\t"
		<< "COH1\tCOH2\n";

	for (size_t i = 0; i < N; ++i) {
		double freq = data["FREQ"][i];
		double omega = 2.0 * PI * freq;  // 

		fout << freq;

		auto write_col = [&](const string& name) {
			if (data.count(name)) {
				if (name == "ZROT" || name == "ZXXR" || name == "ZXXI" || name == "ZXYR" || name == "ZXYI" ||
					name == "ZYXR" || name == "ZYXI" || name == "ZYYR" || name == "ZYYI")
					fout << "\t" << data[name][i] * SI_TRANS;
				else if (name == "ZXX.VAR" || name == "ZXY.VAR" || name == "ZYX.VAR" || name == "ZYY.VAR")
					fout << "\t" << data[name][i] * pow(SI_TRANS, 2);
				else
					fout << "\t" << data[name][i];
			}
			else
				fout << "\tNaN";
			};

		//
		write_col("ZROT");
		write_col("ZXXR");
		write_col("ZXXI");
		write_col("ZXX.VAR");
		write_col("ZXYR");
		write_col("ZXYI");
		write_col("ZXY.VAR");
		write_col("ZYXR");
		write_col("ZYXI");
		write_col("ZYX.VAR");
		write_col("ZYYR");
		write_col("ZYYI");
		write_col("ZYY.VAR");
		write_col("TROT.EXP");
		write_col("TXR.EXP");
		write_col("TXI.EXP");
		write_col("TXVAR.EXP");
		write_col("TYR.EXP");
		write_col("TYI.EXP");
		write_col("TYVAR.EXP");

		// 
		write_col("COH1");
		write_col("COH2");

		fout << "\n";

		// ========== calculate AppRho. Pha. ==========

		auto compute_impedance = [&](const string& real_key, const string& imag_key,
			const string& var_key) -> tuple<double, double, double, double> {
				double app_res = 0.0, phase = 0.0, app_err = 0.0, phase_err = 0.0;

				if (data.count(real_key) && data.count(imag_key)) {
					// toSI
					double z_real = data[real_key][i] * SI_TRANS;
					double z_imag = data[imag_key][i] * SI_TRANS;

					complex<double> z(z_real, z_imag);
					double z_mag = abs(z);
					double z_mag_sq = norm(z);

					if (omega > 0) {

						app_res = z_mag_sq / (omega * MU0);

						phase = atan2(z_imag, z_real) * RAD2DEG;

						if (data.count(var_key) && data[var_key].size() > i) {
							double z_var = data[var_key][i] * pow(SI_TRANS, 2);
							double z_error = sqrt(z_var);

							if (z_mag > 0) {

								app_err = 2.0 * z_mag * z_error / (omega * MU0);

								double tmp = z_error / z_mag;
								double phase_err_rad;
								if (tmp <= 1.0) {
									phase_err_rad = asin(tmp);
								}
								else {
									phase_err_rad = M_PI;
								}
								phase_err = phase_err_rad * RAD2DEG;
							}
						}
					}
				}

				return make_tuple(app_res, phase, app_err, phase_err);
			};

		// 
		auto [app_xx, phase_xx, app_err_xx, phase_err_xx] = compute_impedance("ZXXR", "ZXXI", "ZXX.VAR");
		auto [app_xy, phase_xy, app_err_xy, phase_err_xy] = compute_impedance("ZXYR", "ZXYI", "ZXY.VAR");
		auto [app_yx, phase_yx, app_err_yx, phase_err_yx] = compute_impedance("ZYXR", "ZYXI", "ZYX.VAR");
		auto [app_yy, phase_yy, app_err_yy, phase_err_yy] = compute_impedance("ZYYR", "ZYYI", "ZYY.VAR");

		// 
		fout1 << freq << "\t"
			<< app_xx << "\t" << phase_xx << "\t" << app_err_xx << "\t" << phase_err_xx << "\t"
			<< app_xy << "\t" << phase_xy << "\t" << app_err_xy << "\t" << phase_err_xy << "\t"
			<< app_yx << "\t" << phase_yx << "\t" << app_err_yx << "\t" << phase_err_yx << "\t"
			<< app_yy << "\t" << phase_yy << "\t" << app_err_yy << "\t" << phase_err_yy << "\t";

		if (data.count("COH1") && data["COH1"].size() > i) {
			fout1 << data["COH1"][i] << "\t";
		}
		else {
			fout1 << "NaN\t";
		}

		if (data.count("COH2") && data["COH2"].size() > i) {
			fout1 << data["COH2"][i];
		}
		else {
			fout1 << "NaN";
		}

		fout1 << "\n";
	}

	fout.close();

	cerr << "Total frequencies: " << N << endl;
	//
	cerr << "Data columns found:" << endl;
	for (const auto& item : data) {
		cerr << "  " << item.first << ": " << item.second.size() << " values" << endl;
	}

	return 0;
}
