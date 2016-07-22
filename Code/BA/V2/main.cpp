#define _Use_MATH_DEFINES

#include <cstdlib>
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Timer.hpp"
uint64 whileTime = 0;
int whileIdx = 0;
uint64 trainTime = 0;
int trainIdx = 0;
uint64 statsTime = 0;
int statsIdx = 0;
uint64 focusTime1 = 0;
uint64 focusTime2 = 0;
uint64 focusTime3 = 0;
uint64 focusTime4 = 0;
int focusIdx1 = 0;
int focusIdx2 = 0;
int focusIdx3 = 0;
int focusIdx4 = 0;

#include "SignalSettings.hpp"
#include "SignalBasis.hpp"
#include "Corrupter.hpp"
#include "Signal.hpp"
#include "RVM.hpp"


typedef unsigned int signalType; // Specify C++ type of input signal

int main(int argc, char* argv[])
{
    uint64 startTime = GetTimeMs64(); // start timer

    if (argc < 2) error("No input file specified");
    if (argc > 2) error("To many input arguments");
    SignalSettings cfg(argv[1]);
    
    if(cfg.printProgress) std::cout << cfg << std::endl;
    std::srand(cfg.rngSeed);    

    /*** Memory allocation ***/
    Signal<signalType> signal(cfg.signalDim);
    Signal<signalType> block(cfg.blockDim);        
    
    Signal<signalType> corruptedSignal(signal.dim());
    Signal<bool> sensedEntries(signal.dim());
    Signal<signalType> signalPatch(block.dim());
    Signal<bool> sensedPatch(block.dim());

    Signal<signalType> signalPatchVector(block.size());
    Signal<bool> sensedPatchVector(block.size());
    Signal<bool> initialSensedVector(block.size());
    Signal<signalType> initialSignalVector(block.size());
    Signal<double> recoveredVector(block.size());
    Signal<double> recoveredPatch(block.dim());

    std::vector<Signal<double> > cascadeRecoveredSignals // initialized to zero
    	(cfg.endScale, Signal<double>(signal.dim()));
    std::vector<Signal<double> > cascadeBasis
    	(cfg.endScale, Signal<double>(block.size(), block.size()));

    /*** Program constants ***/
        
    /*** start logic ***/
    signal.read(cfg.inputFile);
    
    if (cfg.simulateCorruption)
	corruptedSignal = corruptSignal(signal, sensedEntries, cfg.corr);
    else {
	sensedEntries.read(cfg.maskFile);
	corruptedSignal = corruptSignal(signal, sensedEntries);
    }
       
    // Get basis matrices for various scales
    for (int scale = 0; scale < cfg.endScale; ++scale)
    	cascadeBasis[scale] = getBasis(block.dim(), cfg.basisMode, scale+1);
    
    // Loop over blocks of original signal
    unsigned int const numBlocksHeight = signal.height() / block.height();
    unsigned int const numBlocksWidth = signal.width() / block.width();
    unsigned int const numBlocksFrames = signal.frames() / block.frames();

    uint64 bigLoopTime = 0;
    uint64 loopSetupTime = 0;
    int itt = 0;
    
    for (int blockIndexRows = 0; blockIndexRows < numBlocksHeight; ++blockIndexRows) {
    	for (int blockIndexCols = 0; blockIndexCols < numBlocksWidth; ++blockIndexCols) {
    	    for (int blockIndexFrames = 0; blockIndexFrames < numBlocksFrames; ++blockIndexFrames) {
    		if (cfg.printProgress) {
    		    std::cout << "\tPatch (" << blockIndexRows+1 << "," 
    			      << blockIndexCols+1 << "," << blockIndexFrames+1
    			      << ")    of    (" << numBlocksHeight << "," 
    			      << numBlocksWidth << "," << numBlocksFrames 
    			      << ")" << std::endl;
    		}
		
		uint64 startLoopTime = GetTimeMs64();
		
    		signalPatch = corruptedSignal
    		    .getPatch(blockIndexRows*block.height(), blockIndexCols*block.width(),
    			      blockIndexFrames*block.frames(), block.height(), block.width(), block.frames());
    		sensedPatch = sensedEntries
    		    .getPatch(blockIndexRows*block.height(), blockIndexCols*block.width(),
			      blockIndexFrames*block.frames(), block.height(), block.width(), block.frames());

		
    		signalPatchVector = vectorize(signalPatch);
    		sensedPatchVector = vectorize(sensedPatch);
    		initialSignalVector = vectorize(signalPatch);
    		initialSensedVector = vectorize(sensedPatch);
		
    		for (int scale = 0; scale < cfg.endScale; ++scale) {		    
    		    int measurements = countSensed(sensedPatchVector);
		    if (measurements == 0) continue;

    		    /*** Declare and define RVM variables ***/
    		    Signal<double> targets = getTargets(signalPatchVector, sensedPatchVector);
    		    Signal<double> designMatrix = getDesignMatrix(cascadeBasis[scale], sensedPatchVector);
		    //Signal<double> errors(block.size()); // for fastUpdates

    		    /*** Start the RVM ***/
    		    bool useCascade;
    		    if (scale+1 < cfg.endScale) useCascade = true;
    		    else useCascade = false;
		
		    uint64 startSetupTime = GetTimeMs64();    
    		    RVM rvm(cfg.stdDev, cfg.deltaML_threshold, cfg.printProgress);

		    rvm.train(designMatrix, targets);		    
		    //rvm.fastUpdates(designMatrix, targets, useCascade, cascadeBasis[scale], errors); // for fastUpdates
		    
		    loopSetupTime += (GetTimeMs64() - startSetupTime);

		    recoveredVector = rvm.predict(cascadeBasis[scale]);		    		    
		    //recoveredVector = matMult(cascadeBasis[scale], rvm.fastMu());		    

		    recoveredVector.fill(initialSignalVector, initialSensedVector);

    		    /*** Save recovered patch ***/
    		    recoveredPatch = reshape(recoveredVector, block.height(), block.width(), block.frames());
		    
    		    cascadeRecoveredSignals[scale]
    			.putPatch(recoveredPatch, blockIndexRows*block.height(),
				  blockIndexCols*block.width(), blockIndexFrames*block.frames());
		    
    		    /*** Prepare for next part of cascade ***/
    		    if (useCascade) {			
			Signal<double> errors = rvm.predictionErrors(cascadeBasis[scale]); // remove for fastupdates
    			for (int i = 0; i < block.size(); ++i) { 
    			    if (errors(i) != 0) sensedPatchVector(i) = true; // get new mask
    			    else sensedPatchVector(i) = false;
			    
    			    // Recovered becomes new signal Patch to get targets for next stage
    			    signalPatchVector(i) = recoveredVector(i); 
    			}
    		    }
		    bigLoopTime += (GetTimeMs64() - startLoopTime);
		    ++itt;
    		}				    
    	    }
    	}
    }

    /*** Output ***/
    outputSignal(corruptedSignal, "corrupted", cfg);
    outputSignal(sensedEntries, "mask", cfg);
    for (int scale = 0; scale < cfg.endScale; ++scale) {
    	std::stringstream label;
    	label << "recovered_" << scale+1 << "_of_" << cfg.endScale;		
	outputSignal(cascadeRecoveredSignals[scale], label.str(), cfg); 
    }
    if (cfg.printProgress) std::cout << "Total execution time: " << GetTimeMs64() - startTime << " ms." << std::endl;
    if (cfg.printProgress) std::cout << "Average loop time: " << bigLoopTime/(double)itt << " ms." << std::endl;
    if (cfg.printProgress) std::cout << "Average  RVM train time: " << loopSetupTime/(double)itt << " ms." << std::endl;
    if (cfg.printProgress) std::cout << "Average train time: " << trainTime/(double)trainIdx << " ms." << std::endl;
    if (cfg.printProgress) std::cout << "Average stats time: " << statsTime/(double)statsIdx << " ms." << std::endl;
    if (cfg.printProgress) std::cout << "Average while loop time: " << whileTime/(double)whileIdx << " ms." << std::endl;
    if (cfg.printProgress) std::cout << "Average focus time 1: " << focusTime1/(double)focusIdx1 << " ms." << std::endl;
    if (cfg.printProgress) std::cout << "Average focus time 2: " << focusTime2/(double)focusIdx2 << " ms." << std::endl;
    if (cfg.printProgress) std::cout << "Average focus time 3: " << focusTime3/(double)focusIdx3 << " ms." << std::endl;
    if (cfg.printProgress) std::cout << "Average focus time 4: " << focusTime4/(double)focusIdx4 << " ms." << std::endl;

    
    return 0;   
}