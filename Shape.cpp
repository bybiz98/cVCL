
#include "stdinc.h"
#include "Shape.hpp"

IMPL_DYN_CLASS(CShape)
CShape::CShape(CComponent* AOwner):CGraphicControl(AOwner),Shape(stRectangle){
	SetControlStyle(GetControlStyle() | csReplicatable);
	SetBounds(GetLeft(), GetTop(), 65, 65);
	Pen = new CPen();
	Pen->SetOnChange(this, (TNotifyEvent)&CShape::StyleChanged);
	Brush = new CBrush();
	Brush->SetOnChange(this, (TNotifyEvent)&CShape::StyleChanged);
}

CShape::~CShape(){
	delete Pen;
	delete Brush;
}

void CShape::Paint(){
	CCanvas* Canvas = GetCanvas();
	Canvas->SetPen(Pen);
	Canvas->SetBrush(Brush);
	INT X = Pen->GetWidth() / 2;
    INT Y = X;
	INT W = GetWidth() - Pen->GetWidth() + 1;
    INT H = GetHeight() - Pen->GetWidth() + 1;
	if(Pen->GetWidth() == 0){
		W--;
		H--;
	}
	INT S = 0;
    if(W < H)
		S = W;
	else 
		S = H;
	if(Shape == stSquare || Shape == stRoundSquare || Shape == stCircle){
		X += (W - S) / 2;
		Y += (H - S) / 2;
		W = S;
		H = S;
	}
	if(Shape == stRectangle || Shape == stSquare)
		Canvas->Rectangle(X, Y, X + W, Y + H);
	else if(Shape == stRoundRect || Shape == stRoundSquare)
		Canvas->RoundRect(X, Y, X + W, Y + H, S / 4, S / 4);
	else if(Shape == stCircle || Shape == stEllipse)
		Canvas->Ellipse(X, Y, X + W, Y + H);
}

void CShape::SetBrush(CBrush* Value){
	Brush->Assign(Value);
}

void CShape::SetPen(CPen* Value){
	Pen->Assign(Value);
}

void CShape::SetShape(TShapeType Value){
	if(Shape != Value){
		Shape = Value;
		Invalidate();
	}
}

void CShape::StyleChanged(CObject* Sender){
	Invalidate();
}