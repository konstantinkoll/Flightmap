// ShareItKeygen.cpp : Definiert den Einstiegspunkt f�r die Konsolenanwendung.
//

#include "stdafx.h"

#define ERC_SUCCESS          0
#define ERC_SUCCESS_BIN      1
#define ERC_ERROR            10
#define ERC_MEMORY           11
#define ERC_FILE_IO          12
#define ERC_BAD_ARGS         13
#define ERC_BAD_INPUT        14
#define ERC_EXPIRED          15
#define ERC_INTENAL          16

#define VERSION "2.0.0"
#define PRODUCT "Flightmap"

char* n = "677085883040394331688570333377767695119671712512083434059528353754560033694591049061201209797395551894503819202786118921144167773531480249549334860535587729188461269633368144074142410191991825789317089507732335118005174575981046999650747204904573316311747574418394100647266984314883856762401810850517725369369312442712786949893638812875664428840233397180906478896311138092374550604342908484026901612764076340393090750130869987901928694525115652071061067946427802582682353995030622395549260092920885717079018306793778696931528069177410572722700379823625160283051668274004965875876083908201130177783610610417898321219849233028817122323965938052450525299474409115105471423275517732060548499857454724731949257103279342856512067188778813745346304689332770001576020711940974480383875829689815572555429459919998181453447896952351950105505906202024278770099672075754601074409510918531448288487849102192484100291069098446047492850214953085906226731086863049147460384108831179220519130075352506339330781986225289808262743848011070853033928165863801245010514393309413470116317612433324938050068689790531474030013439742900179443199754755961937530375097971295589285864719559221786871735111334987792944096096937793086861538051306485745703623856809.";
char* d = "74678590041219963053886433828430260491140262409420966991859744899400003716315189234691309904124509400129097706189645469243842033845383851053235462559072176013433228268386192361118648182940274903233502519235184020368217784115556654373244177011533821652030982472616996394919152681788660672323729137924749121621615342946263266532386633773051223769143389394952920466504904936658957787243703141620614148466626066955120303323257719253889194249093638096072911905856007637795847867098965705391462510248627101148421136778725591573330301747508519050297836009958657384160110471397606530427509254581007004902604111443150550134536450071749986494039517867544515122929751257537582849862580429594057480686119523579965509921792572512634059914423968949178952192606622363973216896980989570688946229033325841108804182143622921852335309271764987205042134381317166703320261730961882343930515583401083519338448479899164193616827444906841933022834898602216657589937068127730180633686784670453719529669211877043710380133345623571308426533023587079117044375611366452875459641990809075602725574428782623402933966081169747393918291680593574768930759603861505547080093812461264272538853296051716320462851556917859742427017792867445326499507628783900968549702193.";
char* e = "17.";

class ShareITData
{
public:
	bool utf8;
	string purchaseId;
	string runningNo;
	string purchaseDate;
	string productId;
	string quantity;
	string regName;
	string language;
};

ShareITData parseInput(char* filename)
{
	string line;

	ifstream input(filename);

	ShareITData result;
	result.utf8 = false;

	if (input.is_open())
		while (!input.eof())
		{
			getline(input, line);

			string::size_type delimiterPos = line.find_first_of("=");

			if (string::npos!=delimiterPos)
			{
				string name = line.substr(0, delimiterPos);
				string value = line.substr(delimiterPos+1);

				if ((name=="ENCODING") && (value=="UTF8"))
				{
					result.utf8 = true;
				}
				else
					if (name=="PURCHASE_ID")
					{
						result.purchaseId = value;
					}
					else
						if (name=="RUNNING_NO")
						{
							result.runningNo = value;
						}
						else
							if (name=="PURCHASE_DATE")
							{
								result.purchaseDate = value;
							}
							else
								if (name=="PRODUCT_ID")
								{
									result.productId = value;
								}
								else
									if (name=="QUANTITY")
									{
										result.quantity = value;
									}
									else
										if (name=="REG_NAME")
										{
											result.regName = value;
										}
			}
		}

	return result;
}

int main(int argc, char* argv[])
{
	if (argc<4)
		return ERC_BAD_ARGS;

	ShareITData input = parseInput(argv[1]);

	///////////////////////////////////////
	// Pseudo Random Number Generator
	AutoSeededRandomPool rng;

	///////////////////////////////////////
	// Create Private Key from key material
	InvertibleRSAFunction params;
	params.Initialize(Integer(n), Integer(e), Integer(d));
	RSA::PrivateKey privateKey(params);

	stringstream ss;
	ss << LICENSE_ID << "=" << input.purchaseId << ":" << input.runningNo << endl;
	ss << LICENSE_DATE << "="<< input.purchaseDate << endl;
	ss << LICENSE_PRODUCT << "="<< PRODUCT << endl;
	ss << LICENSE_QUANTITY << "="<< input.quantity << endl;
	ss << LICENSE_VERSION << "="<< VERSION << endl;
	ss << LICENSE_NAME << "="<< input.regName << endl;
	string message = ss.str();

	////////////////////////////////////////////////
	// Sign and Encode
	RSASS<PSSR, SHA256>::Signer signer(privateKey);

	string signature;
	StringSource(message, true, new SignerFilter(rng, signer, new StringSink(signature)));

	string result;
	StringSource(message+signature, true, new Base64Encoder(new StringSink(result)));

	ofstream keyMetadataOutput(argv[2]);
	keyMetadataOutput << "text/plain:liquidFOLDERS.lic" << endl;
	keyMetadataOutput.close();

	ofstream keyOutput(argv[3]);
	keyOutput << result << endl;
	keyOutput.close();

	return ERC_SUCCESS_BIN;
}
