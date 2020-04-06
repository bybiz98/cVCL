#pragma once

#include "Object.hpp"
#include "Component.hpp"
#include "Messages.hpp"
#include "MsgTarget.hpp"
#include "Types.hpp"
#include "Graphics.hpp"
#include "Control.hpp"
#include "WinControl.hpp"

typedef BYTE TShapeType;
#define stRectangle			0x0
#define stSquare			0x1
#define stRoundRect			0x2
#define stRoundSquare		0x3
#define stEllipse			0x4
#define stCircle			0x5

class cVCL_API CShape : public CGraphicControl{
private:
	CPen* Pen;
	CBrush* Brush;
	TShapeType Shape;
protected:
    void Paint() override;
public:
	CShape(CComponent* AOwner = NULL);
	virtual ~CShape();
	
	DEFINE_GETTER(CBrush*, Brush)
	DEFINE_GETTER(CPen*, Pen)
	DEFINE_GETTER(TShapeType, Shape)

	void SetBrush(CBrush* Value);
	void SetPen(CPen* Value);
	void SetShape(TShapeType Value);
	void StyleChanged(CObject* Sender);

	REF_DYN_CLASS(CShape)
};
DECLARE_DYN_CLASS(CShape, CGraphicControl)