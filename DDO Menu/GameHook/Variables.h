#pragma once


inline bool GameInputs = true;

//Camera 
inline bool isCameraDetach = false;
inline bool LockCamera = false;
//Camera Combat

//Camera Photo Mode
inline bool PhotoModeCheck = false;
inline bool isControlPhotoModeCamera = false;
inline bool isControlCameraRoll = false;
inline bool UnlockNormalCameraLimit = false;
inline bool PhotoModeCameraCollision = false;
inline bool PhotoModeEverywhere = false;
inline float NormalCameraPitchNegativeLimit = -30.0f;
inline float NormalCameraPitchPositiveLimit = 30.0f;
inline float NormalCameraZoom = 400.0f;
inline float NormalCameraY = 180.0f;
inline float NormalCameraX = 0.0f;
inline float NormalCameraFov = 70.0f;

inline float PhotoModeCameraPositiveXLimit = 600.0f; // original 300
inline float PhotoModeCameraNegativeXLimit = -600.0f;
inline float PhotoModeCameraPositiveYLimit = 300.0f; // original 150
inline float PhotoModeCameraNegativeYLimit = -300.0f;

inline float PhotoModePitchPositiveYLimit = 80.0f; //original 30
inline float PhotoModePitchNegativeYLimit = -80.0f; // original 30

inline float PhotoModeCameraY = 0.0f;
inline float PhotoModeCameraX = 0.0f;
inline float PhotoModeCameraZoom = 325.0f;

inline float CameraRoll = 0.0f;

//Character
inline bool isCharacterWeaponOffsetFix = false;
inline float GetCharacterHeight = 0.0f;
inline int GetVocation = 0;
/*
Fighter = 1;
Seeker = 2;
Hunter = 3;
Priest = 4;
Shield Sage = 5;
Sorcerer = 6;
Warrior = 7;
Elemental Archer = 8;
Alchemist = 9;
Spirit Lancer = 10;
High Scepter = 11;
*/

//HUD
inline bool isDisableHud = false;
inline float scaleHudElementX = 0.0f;
inline float scaleHudElementY = 0.0f;
inline float DamageScale = 1.0f;