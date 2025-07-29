#include "/home/codeleaded/System/Static/Library/WindowEngine1.0.h"
#include "/home/codeleaded/System/Static/Library/Random.h"
#include "/home/codeleaded/System/Static/Container/MVector.h"
#include "/home/codeleaded/System/Static/Container/Pair.h"
#include "/home/codeleaded/System/Static/Library/TransformedView.h"
#include "/home/codeleaded/System/Static/Container/SQuadTree.h"

/*
	Vector -> einfach, default Resize: 5
	MVector -> Sehr schnell bei großen datenmengen
*/

typedef struct AObject {
	Vec2 p;
	Vec2 d;
	Vec2 v;
	Pixel c;
} AObject;

TransformedView tv;
MVector Objects;
SQuadTree TreeObjects;
float Area = 100000.0f;
Bool UseTree = False;

void DrawElement(void* Element){
	Pair* pair = (Pair*)Element;
	AObject* obj = (AObject*)Pair_Second(pair);
	Vec2 p = TransformedView_WorldScreenPos(&tv,obj->p);
	Vec2 d = TransformedView_WorldScreenLength(&tv,obj->d);
	RenderRect(p.x,p.y,d.x,d.y,obj->c);
}

void Setup(AlxWindow* w){
	tv = TransformedView_New((Vec2){ 1.0f,1.0f });
    Objects = MVector_New(sizeof(AObject));
	TreeObjects = SQuadTree_New((Rect){ { 0.0f,0.0f },{ Area,Area } },0);

	for(int i = 0;i<1000000;i++){
		//if(i%100000==0) printf("i: %d, %d\n",i,Objects.ExpandSize);
		AObject Obj = {
			{ Random_f64_MinMax(0.0f,Area),Random_f64_MinMax(0.0f,Area) },
			{ Random_f64_MinMax(10.0f,100.0f),Random_f64_MinMax(10.0f,100.0f) },
			{ 0.0f,0.0f },
			Pixel_toRGBA(Random_f64_New(),Random_f64_New(),Random_f64_New(),1.0f)
		};
		MVector_Push(&Objects,(AObject[]){ Obj });
		SQuadTree_Add(&TreeObjects,(AObject[]){ Obj },sizeof(AObject),(Rect){ Obj.p,Obj.d });
	}
	//for(int i = 0;i<Objects.size;i++){
	//	AObject* Obj = (AObject*)MVector_Get(&Objects,i);
	//	SQuadTree_Add(&TreeObjects,Obj,sizeof(AObject),(Rect){ Obj->p,Obj->d });
	//}
}

void Update(AlxWindow* w){
    TransformedView_HandlePanZoom(&tv,window.Strokes,(Vec2){ GetMouse().x,GetMouse().y });
	Rect Screen = TransformedView_Rect(&tv,(Rect){ 0.0f,0.0f,GetWidth(),GetHeight() });
	int ObjectCount = 0;

	if(Stroke(ALX_KEY_TAB).PRESSED){
		UseTree = !UseTree;
	}

	Clear(BLACK);
	
	if(UseTree){
		Timepoint start = Time_Nano();
		List Inside = SQuadTree_Search(&TreeObjects,Screen);
		List_ForEach(&Inside,DrawElement);
		ObjectCount = Inside.size;
		List_Free(&Inside);
		FDuration ElapsedTime = Time_Elapsed(start,Time_Nano());

		String FStr = String_Format("QuadTree / %d in %f",ObjectCount,ElapsedTime);
		//printf("Linear / %d in %f vs %s\n",ObjectCount,ElapsedTime,cstr);
		RenderCStrSize(FStr.Memory,FStr.size,0.0f,0.0f,RED);
		String_Free(&FStr);
	}else{
		Timepoint start = Time_Nano();
		for(int i = 0;i<Objects.size;i++){
			const AObject* obj = (AObject*)MVector_Get(&Objects,i);
			if(Overlap_Rect_Rect(Screen,(Rect){ obj->p,obj->d })){
				Vec2 p = TransformedView_WorldScreenPos(&tv,obj->p);
				Vec2 d = TransformedView_WorldScreenLength(&tv,obj->d);
				RenderRect(p.x,p.y,d.x,d.y,obj->c);
				ObjectCount++;
			}
		}
		FDuration ElapsedTime = Time_Elapsed(start,Time_Nano());

		String FStr = String_Format("Linear / %d in %f",ObjectCount,ElapsedTime);
		//printf("Linear / %d in %f vs %s\n",ObjectCount,ElapsedTime,cstr);
		RenderCStrSize(FStr.Memory,FStr.size,0.0f,0.0f,RED);
		String_Free(&FStr);
	}
}

void Delete(AlxWindow* w){
    MVector_Free(&Objects);
	SQuadTree_Free(&TreeObjects);
}

int main(){
    if(Create("Quad Trees",2500,1300,1,1,Setup,Update,Delete))
        Start();
    return 0;
}