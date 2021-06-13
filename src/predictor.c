//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName1 = "NAME";
const char *studentID1   = "PID";
const char *email1       = "EMAIL";

const char *studentName2 = "Yanbing Fang";
const char *studentID2   = "A59003982";
const char *email2       = "yafang@ucsd.edu";
//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

uint32_t preTabLength; 
uint32_t *preTab; // predictor table
uint32_t gHis; 
uint32_t result;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //

  switch(bpType){
    int i = 0;

    case GSHARE:
      // printf("Start initializing\n");
      
      // printf("ghistoryBitsd:%d\n",ghistoryBits);
      preTabLength = 1 << ghistoryBits;

      preTab = (uint32_t*)malloc( preTabLength*sizeof(uint32_t) );    // predictor table
      for(i = 0; i < preTabLength; i++){    //initialized to WN
        preTab[i] = 1;
      }  

      gHis = 0;  //initialized to NOTTAKEN
      result = 0;
  }


}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      // printf("GSHARE predicting \n");
      // printf("pc: %d\n",pc);

     result = gHis ^ (pc & (preTabLength - 1));
      // printf("result: %d, preTabLength%d\n",result,preTabLength);
      if(preTab[result] > 1){
        return TAKEN;
      }else{
        return NOTTAKEN;
      }

    case TOURNAMENT:
    case CUSTOM:
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //

  switch(bpType) {
    case GSHARE:
      // printf("GSHARE training\n");
      // printf("pc: %d\n",pc);
      
      gHis <<=  1;
      if(outcome == TAKEN){
        gHis |= 1;
        if(preTab[result] < 3){
          preTab[result]++;
        }
      }else{
        if(preTab[result]>0){
          preTab[result]--;
        }
      }

      gHis = gHis & (preTabLength - 1);
  }

}
