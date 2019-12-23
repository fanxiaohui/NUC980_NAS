/*******************************************************************************
 *
 * FILE NAME          : MxL241SF_PhyCfg.cpp
 *
 * AUTHOR             : Brenndon Lee
 * DATE CREATED       : 5/18/2009
 *
 * DESCRIPTION        : This file contains demod and RF control parameters.
 *
 *******************************************************************************
 *                Copyright (c) 2006, MaxLinear, Inc.
 ******************************************************************************/

#include "MxL241SF_PhyCfg.h"

 REG_CTRL_INFO_T MxL_OobAciMfCoef[] =
{
  {0x8044, 0x001F, 0x0000},  /* ACI_COEF_ADDR */
  {0x8046, 0x003F, 0x0000},  /* MF_COEF_ADDR */
  {0x8044, 0x8000, 0x8000},  /* ACI_AUTO_INC */
  {0x8046, 0x8000, 0x8000},  /* MF_AUTO_INC */
  {0,    0,    0}
};

 REG_CTRL_INFO_T MxL_OobAciMfCoef_0_772MHz[] =
{
  {0x8045, 0xFFFF, 0x0004}, /* ACI_COEF_DATA */
  {0x8045, 0xFFFF, 0x0009},
  {0x8045, 0xFFFF, 0x000d},
  {0x8045, 0xFFFF, 0x000f},
  {0x8045, 0xFFFF, 0x000d},
  {0x8045, 0xFFFF, 0x0005},
  {0x8045, 0xFFFF, 0x1ff5},
  {0x8045, 0xFFFF, 0x1fe0},
  {0x8045, 0xFFFF, 0x1fca},
  {0x8045, 0xFFFF, 0x1fb8},
  {0x8045, 0xFFFF, 0x1fb3},
  {0x8045, 0xFFFF, 0x1fc1},
  {0x8045, 0xFFFF, 0x1fe8},
  {0x8045, 0xFFFF, 0x002b},
  {0x8045, 0xFFFF, 0x0085},
  {0x8045, 0xFFFF, 0x00f0},
  {0x8045, 0xFFFF, 0x0160},
  {0x8045, 0xFFFF, 0x01c5},
  {0x8045, 0xFFFF, 0x0212},
  {0x8045, 0xFFFF, 0x023c},

  {0x8047, 0xFFFF, 0x0000}, /* MF_COEF_DATA */
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x01ff},
  {0x8047, 0xFFFF, 0x0002},
  {0x8047, 0xFFFF, 0x0008},
  {0x8047, 0xFFFF, 0x03f3},
  {0x8047, 0xFFFF, 0x03f1},
  {0x8047, 0xFFFF, 0x001e},
  {0x8047, 0xFFFF, 0x0015},
  {0x8047, 0xFFFF, 0x03ca},
  {0x8047, 0xFFFF, 0x03ea},
  {0x8047, 0xFFFF, 0x004f},
  {0x8047, 0xFFFF, 0x0013},
  {0x8047, 0xFFFF, 0x039d},
  {0x8047, 0xFFFF, 0x03f7},
  {0x8047, 0xFFFF, 0x006b},
  {0x8047, 0xFFFF, 0x03f9},
  {0x8047, 0xFFFF, 0x03a0},
  {0x8047, 0xFFFF, 0x001e},
  {0x8047, 0xFFFF, 0x003e},
  {0x8047, 0xFFFF, 0x1fca},
  {0x8047, 0xFFFF, 0x1ffc},
  {0x8047, 0xFFFF, 0x0045},
  {0x8047, 0xFFFF, 0x1fbb},
  {0x8047, 0xFFFF, 0x1fc3},
  {0x8047, 0xFFFF, 0x0086},
  {0x8047, 0xFFFF, 0x0016},
  {0x8047, 0xFFFF, 0x1f5a},
  {0x8047, 0xFFFF, 0x0030},
  {0x8047, 0xFFFF, 0x006c},
  {0x8047, 0xFFFF, 0x1f6e},
  {0x8047, 0xFFFF, 0x004e},
  {0x8047, 0xFFFF, 0x00f4},
  {0x8047, 0xFFFF, 0x1df8},
  {0x8047, 0xFFFF, 0x1ec3},
  {0x8047, 0xFFFF, 0x08f2},
  {0,    0,    0}
};

 REG_CTRL_INFO_T MxL_OobAciMfCoef_1_024MHz[] =
{
  {0x8045, 0xFFFF, 0x1ff3}, /* ACI_COEF_DATA */
  {0x8045, 0xFFFF, 0x1ff5},
  {0x8045, 0xFFFF, 0x1ffd},
  {0x8045, 0xFFFF, 0x000b},
  {0x8045, 0xFFFF, 0x001a},
  {0x8045, 0xFFFF, 0x0025},
  {0x8045, 0xFFFF, 0x0026},
  {0x8045, 0xFFFF, 0x0017},
  {0x8045, 0xFFFF, 0x1ff8},
  {0x8045, 0xFFFF, 0x1fcf},
  {0x8045, 0xFFFF, 0x1fa6},
  {0x8045, 0xFFFF, 0x1f8e},
  {0x8045, 0xFFFF, 0x1f95},
  {0x8045, 0xFFFF, 0x1fc9},
  {0x8045, 0xFFFF, 0x002b},
  {0x8045, 0xFFFF, 0x00b5},
  {0x8045, 0xFFFF, 0x0154},
  {0x8045, 0xFFFF, 0x01ef},
  {0x8045, 0xFFFF, 0x026a},
  {0x8045, 0xFFFF, 0x02ae},

  {0x8047, 0xFFFF, 0x0000}, /* MF_COEF_DATA */
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0001},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x03ff},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0002},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x03fe},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0002},
  {0x8047, 0xFFFF, 0x03ff},
  {0x8047, 0xFFFF, 0x03fe},
  {0x8047, 0xFFFF, 0x03ff},
  {0x8047, 0xFFFF, 0x0002},
  {0x8047, 0xFFFF, 0x0001},
  {0x8047, 0xFFFF, 0x03fd},
  {0x8047, 0xFFFF, 0x03ff},
  {0x8047, 0xFFFF, 0x0007},
  {0x8047, 0xFFFF, 0x0002},
  {0x8047, 0xFFFF, 0x1ff4},
  {0x8047, 0xFFFF, 0x1ffb},
  {0x8047, 0xFFFF, 0x0015},
  {0x8047, 0xFFFF, 0x000b},
  {0x8047, 0xFFFF, 0x1fd1},
  {0x8047, 0xFFFF, 0x1fe9},
  {0x8047, 0xFFFF, 0x0068},
  {0x8047, 0xFFFF, 0x0032},
  {0x8047, 0xFFFF, 0x1f2e},
  {0x8047, 0xFFFF, 0x1f97},
  {0x8047, 0xFFFF, 0x018a},
  {0x8047, 0xFFFF, 0x00b8},
  {0x8047, 0xFFFF, 0x1d07},
  {0x8047, 0xFFFF, 0x1efb},
  {0x8047, 0xFFFF, 0x0968},
  {0,    0,    0}
};

 REG_CTRL_INFO_T MxL_OobAciMfCoef_1_544MHz[] =
{
  {0x8045, 0xFFFF, 0x0002}, /* ACI_COEF_DATA */
  {0x8045, 0xFFFF, 0x0003},
  {0x8045, 0xFFFF, 0x0003},
  {0x8045, 0xFFFF, 0x1ffe},
  {0x8045, 0xFFFF, 0x1ff5},
  {0x8045, 0xFFFF, 0x1fef},
  {0x8045, 0xFFFF, 0x1ff3},
  {0x8045, 0xFFFF, 0x0005},
  {0x8045, 0xFFFF, 0x0022},
  {0x8045, 0xFFFF, 0x0038},
  {0x8045, 0xFFFF, 0x0031},
  {0x8045, 0xFFFF, 0x0000},
  {0x8045, 0xFFFF, 0x1fb1},
  {0x8045, 0xFFFF, 0x1f6b},
  {0x8045, 0xFFFF, 0x1f67},
  {0x8045, 0xFFFF, 0x1fd3},
  {0x8045, 0xFFFF, 0x00b5},
  {0x8045, 0xFFFF, 0x01e1},
  {0x8045, 0xFFFF, 0x02fe},
  {0x8045, 0xFFFF, 0x03ab},

  {0x8047, 0xFFFF, 0x0000}, /* MF_COEF_DATA */
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x0000},
  {0x8047, 0xFFFF, 0x01ff},
  {0x8047, 0xFFFF, 0x0002},
  {0x8047, 0xFFFF, 0x0008},
  {0x8047, 0xFFFF, 0x03f3},
  {0x8047, 0xFFFF, 0x03f1},
  {0x8047, 0xFFFF, 0x001e},
  {0x8047, 0xFFFF, 0x0015},
  {0x8047, 0xFFFF, 0x03ca},
  {0x8047, 0xFFFF, 0x03ea},
  {0x8047, 0xFFFF, 0x004f},
  {0x8047, 0xFFFF, 0x0013},
  {0x8047, 0xFFFF, 0x039d},
  {0x8047, 0xFFFF, 0x03f7},
  {0x8047, 0xFFFF, 0x006b},
  {0x8047, 0xFFFF, 0x03f9},
  {0x8047, 0xFFFF, 0x03a0},
  {0x8047, 0xFFFF, 0x001e},
  {0x8047, 0xFFFF, 0x003e},
  {0x8047, 0xFFFF, 0x1fca},
  {0x8047, 0xFFFF, 0x1ffc},
  {0x8047, 0xFFFF, 0x0045},
  {0x8047, 0xFFFF, 0x1fbb},
  {0x8047, 0xFFFF, 0x1fc3},
  {0x8047, 0xFFFF, 0x0086},
  {0x8047, 0xFFFF, 0x0016},
  {0x8047, 0xFFFF, 0x1f5a},
  {0x8047, 0xFFFF, 0x0030},
  {0x8047, 0xFFFF, 0x006c},
  {0x8047, 0xFFFF, 0x1f6e},
  {0x8047, 0xFFFF, 0x004e},
  {0x8047, 0xFFFF, 0x00f4},
  {0x8047, 0xFFFF, 0x1df8},
  {0x8047, 0xFFFF, 0x1ec3},
  {0x8047, 0xFFFF, 0x08f2},
  {0,    0,    0}
};

 REG_CTRL_INFO_T MxL_OverwriteDefaults[] =
{
  {0x0072, 0xFFFF, 0x0003},
  {0x0099, 0xFFFF, 0x00E7},
  {0x009F, 0xFFFF, 0x0040},
  {0x0084, 0xFFFF, 0x0037},
  {0x0031, 0xFFFF, 0x0002},
  {0x005B, 0xFFFF, 0x0066},
  {0x004E, 0xFFFF, 0x0037},
  {0x003E, 0xFFFF, 0x0007},
  {0x0014, 0xFFFF, 0x0001},
  {0x00A2, 0xFFFF, 0x000B},
  {0x0089, 0xFFFF, 0x0000},
  {0x0088, 0xFFFF, 0x0011},
  {0x00AF, 0xFFFF, 0x002D},
  {0,    0,    0}
};

REG_CTRL_INFO_T MxL_EqualizerSpeedUp[] =
{
  {0x8021, 0xFFFF, 0xD566},
  {0x8050, 0xFFFF, 0x083A},
  {0x8028, 0xFFFF, 0x8508},
  {0,    0,    0}
};

INTERLEAVER_LOOKUP_INFO_T MxL_InterDepthLookUpTable[] =
{
  {128, 1},
  {128, 1},
  {128, 2},
  {64,  2},
  {128, 3},
  {32,  4},
  {128, 4},
  {16,  8},
  {128, 5},
  {8,   16},
  {128, 6},
  {0,   0},
  {128, 7},
  {0,   0},
  {128, 8},
  {0, 0}
};

LUTY_LOOKUP_INFO_T MxL_LutYLookUpTable[] =
{
  {0},
  {0},
  {3010},
  {4771},
  {6021},
  {6990},
  {7782},
  {8451},
  {9031},
  {9542},
  {10000},
  {10414},
  {10792},
  {11139},
  {11461},
  {11761},
  {12041},
  {12304},
  {12553},
  {12788},
  {13010},
  {13222},
  {13424},
  {13617},
  {13802},
  {13979},
  {14150},
  {14314},
  {14472},
  {14624},
  {14771},
  {14914}
};

/*------------------------------------------------------------------------------
--| FUNCTION NAME : Ctrl_ProgramRegisters
--|
--| AUTHOR        : Brenndon Lee
--|
--| DATE CREATED  : 1/15/2008
--|
--| DESCRIPTION   : This function writes multiple registers with provided data array.
--|
--| RETURN VALUE  : None
--|
--|---------------------------------------------------------------------------*/

MXL_STATUS Ctrl_ProgramRegisters(UINT8 I2cSlaveAddr, PREG_CTRL_INFO_T ctrlRegInfoPtr)
{
  UINT8 status = MXL_TRUE;
  UINT16 i, tmp;

  i = 0;

  while (1)
  {
    if ((ctrlRegInfoPtr[i].regAddr == 0) && (ctrlRegInfoPtr[i].mask == 0) && (ctrlRegInfoPtr[i].data == 0)) break;

    // Check if partial bits of register were updated
    if (ctrlRegInfoPtr[i].mask != 0xFFFF)
    {
      status |= Ctrl_ReadRegister(I2cSlaveAddr, ctrlRegInfoPtr[i].regAddr, &tmp);
      if (status != MXL_TRUE) break;;
    }

    tmp &= ~ctrlRegInfoPtr[i].mask;
    tmp |= ctrlRegInfoPtr[i].data;

    status |= Ctrl_WriteRegister(I2cSlaveAddr, ctrlRegInfoPtr[i].regAddr, tmp);
    if (status != MXL_TRUE) break;

    i++;
  }

  return (MXL_STATUS)status;
}

