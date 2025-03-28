#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <cctype>
#include "dsp.h"
#include "generateMusicXML.h"
#include "recordAudio.h"
#include "postprocess.h"
#include "xmlToPDF.h"

#define DEFAULT_OUT "output.xml"
#define DEFAULT_TEST "test/TestingDatasets/Computer-Generated-Samples/D4_to_E5_1_second_per_note.wav"
#define DEFAULT_CLEF "G"
#define DEFAULT_CLEF_LINE 2
#define DEFAULT_TIME_SIG "4/4"
#define DEFAULT_DIVISIONS 480

std::map<std::string, std::string> parsePayload(const std::string& payload) {
    std::map<std::string, std::string> result;

    // Debugging: output full payload
    std::cout << "Received Payload: " << payload << std::endl;

    // Find the first '{' and the last '}' to isolate the body.
    size_t start = payload.find('{');
    size_t end = payload.rfind('}');

    if (start == std::string::npos || end == std::string::npos || end <= start) {
        std::cout << "Warning: Invalid payload format" << std::endl;
        return result;
    }

    std::string body = payload.substr(start + 1, end - start - 1);

    std::istringstream iss(body);
    std::string token;
    while (std::getline(iss, token, ',')) {
        size_t colonPos = token.find(':');
        if (colonPos != std::string::npos) {
            std::string key = token.substr(0, colonPos);
            std::string value = token.substr(colonPos + 1);

            // Trim whitespace and quotes
            auto trim = [](std::string s) {
                // remove leading/trailing spaces
                s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                    return !std::isspace(ch);
                    }));
                s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                    return !std::isspace(ch);
                    }).base(), s.end());

                // remove surrounding quotes if any
                if (!s.empty() && (s.front() == '\"' || s.front() == '\''))
                    s.erase(0, 1);
                if (!s.empty() && (s.back() == '\"' || s.back() == '\''))
                    s.pop_back();

                return s;
                };

            key = trim(key);
            value = trim(value);

            // Debugging: output each parsed key-value pair
            std::cout << "Parsed: Key='" << key << "', Value='" << value << "'" << std::endl;

            result[key] = value;
        }
    }

    return result;
}

void processAudio(const std::map<std::string, std::string>& payload) {
    try {
        // Check if temp.wav exists
        std::ifstream tempWav("temp.wav");
        if (!tempWav.good()) {
            std::cout << "Failed to generate MusicXML file. Temp WAV file not found." << std::endl;
            return;
        }

        DSPResult res = dsp("temp.wav");

        // Use default values if keys are missing
        std::string workNumber = payload.count("workNumber") ? payload.at("workNumber") : "Untitled Work";
        std::string workTitle = payload.count("workTitle") ? payload.at("workTitle") : "Untitled";
        std::string movementNumber = payload.count("movementNumber") ? payload.at("movementNumber") : "1";
        std::string movementTitle = payload.count("movementTitle") ? payload.at("movementTitle") : "Movement";
        std::string creatorName = payload.count("creatorName") ? payload.at("creatorName") : "Anonymous";
        std::string instrument = payload.count("instrumentInput") ? payload.at("instrumentInput") : "Piano";
        std::string timeSignature = payload.count("timeSignatureInput") ? payload.at("timeSignatureInput") : DEFAULT_TIME_SIG;

        // Log the values being used
        std::cout << "Processing Audio with following parameters:" << std::endl;
        std::cout << "Work Number: " << workNumber << std::endl;
        std::cout << "Work Title: " << workTitle << std::endl;
        std::cout << "Movement Number: " << movementNumber << std::endl;
        std::cout << "Movement Title: " << movementTitle << std::endl;
        std::cout << "Creator: " << creatorName << std::endl;
        std::cout << "Instrument: " << instrument << std::endl;
        std::cout << "Time Signature: " << timeSignature << std::endl;

        MusicXMLGenerator xmlGenerator(workNumber, workTitle, movementNumber, movementTitle, creatorName, instrument, timeSignature);
        bool success = xmlGenerator.generate(
            DEFAULT_OUT,
            res.XMLNotes,
            DEFAULT_CLEF,
            DEFAULT_CLEF_LINE,
            res.keySignature,
            DEFAULT_DIVISIONS
        );

        if (success) {
            postProcessMusicXML(DEFAULT_OUT, DEFAULT_OUT);
            std::cout << "MusicXML file generated successfully." << std::endl;
        }
        else {
            std::cout << "Failed to generate MusicXML file." << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cout << "Error in processAudio: " << e.what() << std::endl;
        std::cout << "Failed to generate MusicXML file." << std::endl;
    }
}

void generatePDF(const std::map<std::string, std::string>& payload) {
    processAudio(payload);
    convertMusicXMLToPDF(DEFAULT_OUT, "output.pdf");
}

int main() {
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string command;
        iss >> command;
        std::string payloadStr;
        std::getline(iss, payloadStr); // Get the rest of the line as the payload string

        // Use our simple parser
        std::map<std::string, std::string> payload = parsePayload(payloadStr);

        if (command == "processAudio") {
            processAudio(payload);
        }
        else if (command == "generatePDF") {
            generatePDF(payload);
        }
        else {
            std::cerr << "Unknown command: " << command << std::endl;
        }
    }
    return 0;
}