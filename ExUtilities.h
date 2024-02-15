#pragma once
#include "A3DSDKIncludes.h"

bool SearchBrepModelFromTopoBrep(A3DAsmModelFile* pModelFile, A3DTopoBrepData* pTopoBrep, A3DRiBrepModel*& pRiBrepModel);
bool PkBodyToA3DRiBrepModel(int body, A3DRiBrepModel*& pRiBrepModel, A3DMiscPKMapper*& pPkMapper);
