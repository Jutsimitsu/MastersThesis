#pragma once
#ifndef _BONEDATA_H_
#define _BONEDATA_H_

#include<windows.h>
#include <iostream>
#include <D3DX11.h>
#include <D3DX10math.h>

using namespace std;

class BoneData
{
private: 
	int id;
	std::string name;
	int parentID;
	D3DXVECTOR3 localPosition;
	D3DXQUATERNION localOrientation;
	D3DXMATRIX transformation;

public:
	BoneData(void);
	BoneData(const BoneData&);
	~BoneData(void);

	void SetID(int _id) { id = _id; }
	void SetName(std::string _name) { name = _name; }
	void SetParent(int _id) { parentID = _id; }
	void SetLocalPos(D3DXVECTOR3 localPos) { localPosition = localPos; }
	void SetLocalOrient(D3DXQUATERNION localOrient) { localOrientation = localOrient; }
	void SetTransformation(D3DXMATRIX transform) { transformation = transform; }

	int GetID() { return id; }
	int GetParent() { return parentID; }
	std::string GetName() { return name; }
	D3DXVECTOR3 GetLocalPos() { return localPosition; }
	D3DXQUATERNION GetLocalOrient() { return localOrientation; }
	D3DXMATRIX GetTransformation() { return transformation; }
};

#endif