//3DMenu. Author: SDraw
//Original posts are on forum.sa-mp.com, pawno.ru
/* Fake natives. Thanks to TheArcher.
native Create3DMenu(Float:x,Float:y,Float:z,Float:rotation,boxes,playerid);
native SetBoxText(MenuID,box,text[],materialsize,fontface[],fontsize,bold,fontcolor,selectcolor,unselectcolor,textalignment);
native Select3DMenu(playerid,MenuID);
native CancelSelect3DMenu(playerid);
native Destroy3DMenu(MenuID);
*/

#include <YSI_Coding\y_hooks>

#define INVALID_3DMENU  (0xFFFF)

#define MAX_3DMENUS (MAX_PLAYERS)
#define MAX_BOXES (16)

new SelectedMenu[MAX_PLAYERS];
new SelectedBox[MAX_PLAYERS];

enum MenuParams
{
	Float:MenuRotation,
	Boxes,
	bool:IsExist,
	Objects[MAX_BOXES],
	Float:OrigPosX[MAX_BOXES],
	Float:OrigPosY[MAX_BOXES],
	Float:OrigPosZ[MAX_BOXES],
	Float:AddingX,
	Float:AddingY,
	SelectColor[MAX_BOXES],
	UnselectColor[MAX_BOXES],
	Player
}

new MenuInfo[MAX_3DMENUS][MenuParams];

//Callbacks
forward OnPlayerSelect3DMenuBox(playerid,MenuID,boxid);
forward OnPlayerChange3DMenuBox(playerid,MenuID,boxid);

// Create a new menu
stock Create3DMenu(playerid,Float:x,Float:y,Float:z,Float:rotation,boxes)
{
	// Make sure box is in range
	if(boxes > MAX_BOXES || boxes <= 0) return -1;

	// Create 3D Menu
	for(new i = 0; i < MAX_3DMENUS; i++)
	{
		// Menu exists continue
	    if(MenuInfo[i][IsExist]) continue;

     	new Float:NextLineX,Float:NextLineY;
      	new lineindx,binc;

       	MenuInfo[i][MenuRotation] = rotation;
		MenuInfo[i][Boxes] = boxes;
		MenuInfo[i][AddingX] = 0.25*floatsin(rotation,degrees);
		MenuInfo[i][AddingY] = -floatcos(rotation,degrees)*0.25;

		NextLineX = floatcos(rotation,degrees)+0.05*floatcos(rotation,degrees);
		NextLineY = floatsin(rotation,degrees)+0.05*floatsin(rotation,degrees);

		// Create menu objects
		for(new b = 0; b < boxes; b++)
		{
  			if(b%4 == 0 && b != 0) lineindx++,binc+=4;
   			MenuInfo[i][Objects][b] = CreateDynamicObject(2661,x+NextLineX*lineindx,y+NextLineY*lineindx,z+1.65-0.55*(b-binc),0,0,rotation,-1,-1,playerid,100.0);
      		GetDynamicObjectPos(MenuInfo[i][Objects][b],MenuInfo[i][OrigPosX][b],MenuInfo[i][OrigPosY][b],MenuInfo[i][OrigPosZ][b]);
		}
		MenuInfo[i][IsExist] = true;
		MenuInfo[i][Player] = playerid;
		Streamer_Update(playerid);
		return i;
	}
	return -1;
}

stock SetBoxMaterial(MenuID,box,index,model,txd[],texture[], selectcolor, unselectcolor)
{
	if(!MenuInfo[MenuID][IsExist]) return -1;
	if(box == MenuInfo[MenuID][Boxes] || box < 0) return -1;
	if(MenuInfo[MenuID][Objects][box] == INVALID_OBJECT_ID) return -1;
	MenuInfo[MenuID][SelectColor][box] = selectcolor;
	MenuInfo[MenuID][UnselectColor][box] = unselectcolor;
	if(SelectedBox[MenuInfo[MenuID][Player]] == box) SetDynamicObjectMaterial(MenuInfo[MenuID][Objects][box], index, model, txd, texture, selectcolor);
	else SetDynamicObjectMaterial(MenuInfo[MenuID][Objects][box], index, model, txd, texture, unselectcolor);
	return 1;
}

stock Select3DMenu(playerid,MenuID)
{
	if(!IsPlayerConnected(playerid)) return -1;
	if(!MenuInfo[MenuID][IsExist]) return -1;
	if(MenuInfo[MenuID][Player] != playerid) return -1;
	if(SelectedMenu[playerid] != -1) CancelSelect3DMenu(playerid);

	SelectedBox[playerid] = 0;
	SelectedMenu[playerid] = MenuID;

	new model,txd[32],texture[32], color;
	GetDynamicObjectMaterial(MenuInfo[MenuID][Objects][0],0,model, txd, texture, color);
 	SetDynamicObjectMaterial(MenuInfo[MenuID][Objects][0],0, model, txd, texture, MenuInfo[MenuID][SelectColor][0]);

 	MoveDynamicObject(MenuInfo[MenuID][Objects][0],MenuInfo[MenuID][OrigPosX][0]+MenuInfo[MenuID][AddingX],MenuInfo[MenuID][OrigPosY][0]+MenuInfo[MenuID][AddingY],MenuInfo[MenuID][OrigPosZ][0],1.0);

	return 1;
}

hook OnGameModeInit()
{
	for(new i = 0; i < MAX_3DMENUS; i++)
	{
	    for(new b = 0; b < MAX_BOXES; b++) MenuInfo[i][Objects][b] = INVALID_OBJECT_ID;
	    MenuInfo[i][Boxes] = 0;
	    MenuInfo[i][IsExist] = false;
	    MenuInfo[i][AddingX] = 0.0;
 	    MenuInfo[i][AddingY] = 0.0;
 	    MenuInfo[i][Player] = -1;
	}
	return 1;
}

hook OnPlayerConnect(playerid)
{
    SelectedMenu[playerid] = -1;
	SelectedBox[playerid] = -1;
	return 1;
}

hook OnPlayerDisconnect(playerid,reason)
{
    if(SelectedMenu[playerid] != -1) CancelSelect3DMenu(playerid);
    return 1;
}


hook OnPlayerKeyStateChange(playerid,newkeys,oldkeys)
{
	if(SelectedMenu[playerid] != -1)
	{
		new MenuID = SelectedMenu[playerid];

		if(OnPlayerKeyStateChangeMenu(playerid,newkeys,oldkeys)) return 1;


	    if(newkeys == KEY_NO)
	    {
			new model,txd[32],texture[32], color;
			GetDynamicObjectMaterial(MenuInfo[MenuID][Objects][SelectedBox[playerid]],0,model, txd, texture, color);
		 	SetDynamicObjectMaterial(MenuInfo[MenuID][Objects][SelectedBox[playerid]],0, model, txd, texture, MenuInfo[MenuID][UnselectColor][SelectedBox[playerid]]);

			MoveDynamicObject(MenuInfo[MenuID][Objects][SelectedBox[playerid]],MenuInfo[MenuID][OrigPosX][SelectedBox[playerid]],MenuInfo[MenuID][OrigPosY][SelectedBox[playerid]],MenuInfo[MenuID][OrigPosZ][SelectedBox[playerid]],1.0);
			SelectedBox[playerid]++;

			if(SelectedBox[playerid] == MenuInfo[MenuID][Boxes]) SelectedBox[playerid] = 0;

			GetDynamicObjectMaterial(MenuInfo[MenuID][Objects][SelectedBox[playerid]],0,model, txd, texture, color);
		 	SetDynamicObjectMaterial(MenuInfo[MenuID][Objects][SelectedBox[playerid]],0, model, txd, texture, MenuInfo[MenuID][SelectColor][SelectedBox[playerid]]);

			MoveDynamicObject(MenuInfo[MenuID][Objects][SelectedBox[playerid]],MenuInfo[MenuID][OrigPosX][SelectedBox[playerid]]+MenuInfo[MenuID][AddingX],MenuInfo[MenuID][OrigPosY][SelectedBox[playerid]]+MenuInfo[MenuID][AddingY],MenuInfo[MenuID][OrigPosZ][SelectedBox[playerid]],1.0);

		}
		if(newkeys == KEY_YES)
	    {
			new model,txd[32],texture[32], color;
			GetDynamicObjectMaterial(MenuInfo[MenuID][Objects][SelectedBox[playerid]],0,model, txd, texture, color);
		 	SetDynamicObjectMaterial(MenuInfo[MenuID][Objects][SelectedBox[playerid]],0, model, txd, texture, MenuInfo[MenuID][UnselectColor][SelectedBox[playerid]]);

	        MoveDynamicObject(MenuInfo[MenuID][Objects][SelectedBox[playerid]],MenuInfo[MenuID][OrigPosX][SelectedBox[playerid]],MenuInfo[MenuID][OrigPosY][SelectedBox[playerid]],MenuInfo[MenuID][OrigPosZ][SelectedBox[playerid]],1.0);
			SelectedBox[playerid]--;

			if(SelectedBox[playerid] < 0) SelectedBox[playerid] = MenuInfo[MenuID][Boxes]-1;

			GetDynamicObjectMaterial(MenuInfo[MenuID][Objects][SelectedBox[playerid]],0,model, txd, texture, color);
		 	SetDynamicObjectMaterial(MenuInfo[MenuID][Objects][SelectedBox[playerid]],0, model, txd, texture, MenuInfo[MenuID][SelectColor][SelectedBox[playerid]]);

			MoveDynamicObject(MenuInfo[MenuID][Objects][SelectedBox[playerid]],MenuInfo[MenuID][OrigPosX][SelectedBox[playerid]]+MenuInfo[MenuID][AddingX],MenuInfo[MenuID][OrigPosY][SelectedBox[playerid]]+MenuInfo[MenuID][AddingY],MenuInfo[MenuID][OrigPosZ][SelectedBox[playerid]],1.0);
		}
		

		if(newkeys == KEY_NO || newkeys == KEY_YES) if(funcidx("OnPlayerChange3DMenuBox") != -1) CallLocalFunction("OnPlayerChange3DMenuBox","ddd",playerid,MenuID,SelectedBox[playerid]);
		if(newkeys == KEY_WALK) if(funcidx("OnPlayerSelect3DMenuBox") != -1) CallLocalFunction("OnPlayerSelect3DMenuBox","ddd",playerid,MenuID,SelectedBox[playerid]);
	}
	return 1;
}

stock CancelSelect3DMenu(playerid)
{
	if(!IsPlayerConnected(playerid)) return -1;
	if(SelectedMenu[playerid] == -1) return -1;
	new MenuID = SelectedMenu[playerid];

	new model,txd[32],texture[32], color;
	GetDynamicObjectMaterial(MenuInfo[MenuID][Objects][SelectedBox[playerid]],0,model, txd, texture, color);
 	SetDynamicObjectMaterial(MenuInfo[MenuID][Objects][SelectedBox[playerid]],0, model, txd, texture, MenuInfo[MenuID][UnselectColor][SelectedBox[playerid]]);


	MoveDynamicObject(MenuInfo[MenuID][Objects][SelectedBox[playerid]],MenuInfo[MenuID][OrigPosX][SelectedBox[playerid]],MenuInfo[MenuID][OrigPosY][SelectedBox[playerid]],MenuInfo[MenuID][OrigPosZ][SelectedBox[playerid]],1.0);
	SelectedMenu[playerid] = -1;
	SelectedBox[playerid] = -1;
	return 1;
}

stock Destroy3DMenu(MenuID)
{
    if(!MenuInfo[MenuID][IsExist]) return -1;
    if(SelectedMenu[MenuInfo[MenuID][Player]] == MenuID) CancelSelect3DMenu(MenuInfo[MenuID][Player]);
    for(new i = 0; i < MenuInfo[MenuID][Boxes]; i++)
    {
		DestroyDynamicObject(MenuInfo[MenuID][Objects][i]);
		MenuInfo[MenuID][Objects][i] = INVALID_OBJECT_ID;
	}
 	MenuInfo[MenuID][Boxes] = 0;
 	MenuInfo[MenuID][IsExist] = false;
 	MenuInfo[MenuID][AddingX] = 0.0;
 	MenuInfo[MenuID][AddingY] = 0.0;
 	MenuInfo[MenuID][Player] = -1;
	return 1;
}
