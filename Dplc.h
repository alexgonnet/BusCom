/*
 * Dplc.h
 *
 *  Created on: 14 nov. 2019
 *      Author: 8701984
 */

#ifndef DPLC_H_
#define DPLC_H_
void Set_vitesse_Mot_1 (int vitss1);
int Get_vitesse_Mot_1();
void Set_vitesse_Mot_2 (int vitss2);
int Get_vitesse_Mot_2();
void Set_vitesse (int vitss1, int vitss2);
void Set_Direction (char dir);
void Init_robot();
void Equilibre();
int val_optocoupleur_Mot_1();
int val_optocoupleur_Mot_2();
int Lecture_nbr_tour(int nbr);

#endif /* DPLC_H_ */
