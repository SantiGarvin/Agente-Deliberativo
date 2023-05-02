#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>

using Nodo = ComportamientoJugador::Nodo;
using Grafo = ComportamientoJugador::Grafo;
using Celda = ComportamientoJugador::Celda;
using Mapa = ComportamientoJugador::Mapa;
using Terreno = ComportamientoJugador::Terreno;
using Entidad = ComportamientoJugador::Entidad;
using Estado = ComportamientoJugador::Estado;

// Este es el método principal que se piden en la practica.
// Tiene como entrada la información de los sensores y devuelve la acción a realizar.
// Para ver los distintos sensores mirar fichero "comportamiento.hpp"
Action ComportamientoJugador::think(Sensores sensores)
{
	Action accion = actIDLE;

	// Actualizar información de los sensores
	actualizaEstado(sensores);

	if (sensores.nivel != 4)
	{
		// Verificar si es el inicio de la simulación, si hay un reinicio o si es la primera iteración
		if (sensores.reset || primeraIteracion) {
			primeraIteracion = false; // Cambiar el valor de primeraIteracion a false para que no entre en el if en futuras iteraciones
			return actWHEREIS;
		}

		actualizaMapaAux();
		actualizaGrafo(mapaAux);

		if (!hayPlan)
		{
			cout << "Calculando un nuevo plan\n";

			ubicacion destino = {sensores.destinoF, sensores.destinoC};
			
			switch (sensores.nivel)
			{
			case 0:
				plan = busquedaAnchuraJugador(estadoActual, destino);
				break;
				// case 1:
				// 	plan = busquedaAnchuraSonambulo();
				// 	break;
				// case 2:
				// 	plan = encuentraCaminoDijkstraJugador();
				// 	break;
				// case 3:
				// 	plan = encuentraCaminoAStarSonambulo();
				// 	break;
				// case 4:
				// 	plan = maximizarPuntuacion();
				// 	break;
			}
			if (plan.size() > 0)
			{
				visualizaPlan(plan);
				hayPlan = true;
			}
		}
		if (hayPlan && plan.size() > 0)
		{
			cout << "Ejecutando la siguiente accion del plan\n";
			accion = plan.front();
			plan.pop_front();
		}
		if (plan.size() == 0)
		{
			cout << "Se completo el plan\n";
			hayPlan = false;
		}
	}
	else
	{
		actualizaMapaVisionJugador(sensores);
		actualizaGrafo(mapaAux);

		// ------------------
		// COMPLETAR NIVEL 4
		// ------------------
	}

	return accion;
}

void ComportamientoJugador::actualizaEstado(const Sensores &sensores)
{
	nivel = sensores.nivel;

	if (sensores.nivel != 4)
	{
		// Actualizar posición del jugador
		estadoActual.jugador.f = sensores.posF;
		estadoActual.jugador.c = sensores.posC;
		estadoActual.jugador.brujula = sensores.sentido;

		// Actualizar posición del sonámbulo
		estadoActual.sonambulo.f = sensores.SONposF;
		estadoActual.sonambulo.c = sensores.SONposC;
		estadoActual.sonambulo.brujula = sensores.SONsentido;

		int f = estadoActual.jugador.f;
		int c = estadoActual.jugador.c;

		// Actualizar lista de sonámbulos
		sonambulos.clear();
		for (int i = 0; i < mapaEntidades.size(); ++i)
			for (int j = 0; j < mapaEntidades[0].size(); ++j)
				if (mapaEntidades[i][j] == Entidad::Sonambulo)
					sonambulos.push_back({i, j});

		// Actualizar lista de aldeanos
		aldeanos.clear();
		for (int i = 0; i < mapaEntidades.size(); ++i)
			for (int j = 0; j < mapaEntidades[0].size(); ++j)
				if (mapaEntidades[i][j] == Entidad::Aldeano)
					aldeanos.push_back({i, j});

		// Actualizar lista de lobos
		lobos.clear();
		for (int i = 0; i < mapaEntidades.size(); i++)
			for (int j = 0; j < mapaEntidades[0].size(); j++)
				if (mapaEntidades[i][j] == Entidad::Lobo)
					lobos.push_back({i, j});
	}

	int f = estadoActual.jugador.f;
	int c = estadoActual.jugador.c;

	if (mapaAux[f][c].terreno == Terreno::Bikini && !tieneBikini && !tieneZapatillas)
		tieneBikini = true;
	else
		tieneBikini = false;

	if (mapaAux[f][c].terreno == Terreno::Zapatillas && !tieneZapatillas && !tieneBikini)
		tieneZapatillas = true;
	else
		tieneZapatillas = false;

	tiempo = sensores.tiempo;
}

void ComportamientoJugador::actualizaPosicionOrientacion(bool esJugador)
{
	if (esJugador)
	{
		switch (ultimaAccionJugador)
		{
		case actFORWARD:
			switch (estadoActual.jugador.brujula)
			{
			case norte:
				estadoActual.jugador.f--;
				break;
			case noreste:
				estadoActual.jugador.f--;
				estadoActual.jugador.c++;
				break;
			case este:
				estadoActual.jugador.c++;
				break;
			case sureste:
				estadoActual.jugador.f++;
				estadoActual.jugador.c++;
				break;
			case sur:
				estadoActual.jugador.f++;
				break;
			case suroeste:
				estadoActual.jugador.f++;
				estadoActual.jugador.c--;
				break;
			case oeste:
				estadoActual.jugador.c--;
				break;
			case noroeste:
				estadoActual.jugador.f--;
				estadoActual.jugador.c--;
				break;
			}
			break;
		case actTURN_R: // giro a la izquierda 90 grados
			estadoActual.jugador.brujula = static_cast<Orientacion>((estadoActual.jugador.brujula + 2) % 8);
			break;
		case actTURN_L: // giro a la derecha 90 grados
			estadoActual.jugador.brujula = static_cast<Orientacion>((estadoActual.jugador.brujula + 6) % 8);
			break;
		}
	}
	else // es sonambulo
	{
		switch (ultimaAccionSonambulo)
		{
		case actSON_FORWARD:
			switch (estadoActual.sonambulo.brujula)
			{

			case norte:
				estadoActual.sonambulo.f--;
				break;
			case noreste:
				estadoActual.sonambulo.f--;
				estadoActual.sonambulo.c++;
				break;
			case este:
				estadoActual.sonambulo.c++;
				break;
			case sureste:
				estadoActual.sonambulo.f++;
				estadoActual.sonambulo.c++;
				break;
			case sur:
				estadoActual.sonambulo.f++;
				break;
			case suroeste:
				estadoActual.sonambulo.f++;
				estadoActual.sonambulo.c--;
				break;
			case oeste:
				estadoActual.sonambulo.c--;
				break;
			case noroeste:
				estadoActual.sonambulo.f--;
				estadoActual.sonambulo.c--;
				break;
			}
			break;
		case actTURN_SR: // giro a la derecha 45 grados
			estadoActual.sonambulo.brujula = static_cast<Orientacion>((estadoActual.jugador.brujula + 1) % 8);
			break;
		case actTURN_SL: // giro a la izquierda 45 grados
			estadoActual.sonambulo.brujula = static_cast<Orientacion>((estadoActual.jugador.brujula + 7) % 8);
			break;
		}
	}
}

void ComportamientoJugador::actualizaMapaAux()
{
	for (int i = 0; i < mapaAux.size(); ++i)
	{
		for (int j = 0; j < mapaAux[0].size(); ++j)
		{
			mapaAux[i][j].terreno = charToTerreno(mapaResultado[i][j]);
			mapaAux[i][j].superficie = charToEntidad(mapaEntidades[i][j]);
		}
	}
}

void ComportamientoJugador::actualizaGrafo(const Mapa &mapaAux)
{
	int filas = mapaAux.size();
	int columnas = mapaAux[0].size();

	// Coordenadas de desplazamiento: arriba, derecha, abajo, izquierda, y las diagonales.
	int dfil[] = {-1, 0, 1, 0, -1, 1, 1, -1};
	int dcol[] = {0, 1, 0, -1, 1, 1, -1, -1};

	for (int f = 0; f < filas; ++f)
	{
		for (int c = 0; c < columnas; ++c)
		{
			grafo[f][c].celda.terreno = charToTerreno(mapaAux[f][c].terreno);
			grafo[f][c].celda.superficie = charToEntidad(mapaAux[f][c].superficie);
		}
	}
}

Terreno ComportamientoJugador::charToTerreno(unsigned char c)
{
	switch (c)
	{
	case 'B':
		return Terreno::Bosque;
	case 'A':
		return Terreno::Agua;
	case 'P':
		return Terreno::Precipicio;
	case 'S':
		return Terreno::SueloPiedra;
	case 'T':
		return Terreno::SueloArenoso;
	case 'M':
		return Terreno::Muro;
	case 'K':
		return Terreno::Bikini;
	case 'D':
		return Terreno::Zapatillas;
	case 'X':
		return Terreno::Recarga;
	default:
		return Terreno::Desconocido;
	}
}

Entidad ComportamientoJugador::charToEntidad(unsigned char c)
{
	switch (c)
	{
	case 'j':
		return Entidad::Jugador;
	case 's':
		return Entidad::Sonambulo;
	case 'a':
		return Entidad::Aldeano;
	case 'l':
		return Entidad::Lobo;
	default:
		return Entidad::SinEntidad;
	}
}

void ComportamientoJugador::inicializaVariablesEstado()
{
	nivel = -1;
	primeraIteracion = true;
	estadoActual.jugador.f = 99;
	estadoActual.jugador.c = 99;
	estadoActual.jugador.brujula = norte;
	estadoActual.sonambulo.f = 99;
	estadoActual.sonambulo.c = 99;
	estadoActual.sonambulo.brujula = norte;
	tieneBikini = false;
	tieneZapatillas = false;

	tiempo = 0;
	hayPlan = false;

	ultimaAccionJugador = actIDLE;
	ultimaAccionSonambulo = actIDLE;
}

// void ComportamientoJugador::inicializarGrafo(int tamanio)
// {
// 	for (int i = 0; i < tamanio; ++i)
// 	{
// 		for (int j = 0; j < tamanio; ++j)
// 		{
// 			grafo[i][j] = Nodo(i, j, Orientacion::norte);
// 		}
// 	}
// }

void ComportamientoJugador::anularMatriz(vector<vector<unsigned char>> &matriz)
{
	for (int i = 0; i < matriz.size(); ++i)
	{
		for (int j = 0; j < matriz.size(); ++j)
		{
			matriz[i][j] = 0;
		}
	}
}

void ComportamientoJugador::visualizaPlan(const list<Action> &plan)
{
	anularMatriz(mapaConPlan);
	Estado cst = estadoActual;

	auto it = plan.begin();
	while (it != plan.end())
	{
		switch (*it)
		{
		case actFORWARD:
			cst.jugador = siguienteCasilla(cst.jugador);
			mapaConPlan[cst.jugador.f][cst.jugador.c] = 1;
		case actTURN_R:
			cst.jugador.brujula = (Orientacion)((cst.jugador.brujula + 2) % 8);
			break;
		case actTURN_L:
			cst.jugador.brujula = (Orientacion)((cst.jugador.brujula + 6) % 8);
			break;
		case actSON_FORWARD:
			cst.sonambulo = siguienteCasilla(cst.sonambulo);
			mapaConPlan[cst.sonambulo.f][cst.sonambulo.c] = 2;
			break;
		case actSON_TURN_SR:
			cst.sonambulo.brujula = (Orientacion)((cst.sonambulo.brujula + 1) % 8);
			break;
		case actSON_TURN_SL:
			cst.sonambulo.brujula = (Orientacion)((cst.sonambulo.brujula + 7) % 8);
			break;
		}
		it++;
	}
}

ubicacion ComportamientoJugador::siguienteCasilla(const ubicacion &pos)
{
	ubicacion next = pos;

	switch (pos.brujula)
	{
	case norte:
		next.f = pos.f - 1;
		break;
	case noreste:
		next.f = pos.f - 1;
		next.c = pos.c + 1;
		break;
	case este:
		next.c = pos.c + 1;
		break;
	case sureste:
		next.f = pos.f + 1;
		next.c = pos.c + 1;
		break;
	case sur:
		next.f = pos.f + 1;
		break;
	case suroeste:
		next.f = pos.f + 1;
		next.c = pos.c - 1;
		break;
	case oeste:
		next.c = pos.c - 1;
		break;
	case noroeste:
		next.f = pos.f - 1;
		next.c = pos.c - 1;
		break;
	}
	return next;
}

Estado ComportamientoJugador::aplicar(const Action &a, const Estado &est)
{
	Estado estadoResult = est;
	ubicacion sigUbicacion;

	switch (a)
	{
	case actFORWARD:
		sigUbicacion = siguienteCasilla(estadoResult.jugador);
		if (casillaTransitable(sigUbicacion) && !(sigUbicacion.f == estadoActual.sonambulo.f && sigUbicacion.c == estadoActual.sonambulo.c))
			estadoResult.jugador = sigUbicacion;
		break;
	case actTURN_R:
		estadoResult.jugador.brujula = (Orientacion)((estadoResult.jugador.brujula + 2) % 8);
		break;
	case actTURN_L:
		estadoResult.jugador.brujula = (Orientacion)((estadoResult.jugador.brujula + 6) % 8);
		break;
	case actSON_FORWARD:
		sigUbicacion = siguienteCasilla(estadoResult.sonambulo);
		if (casillaTransitable(sigUbicacion) && !(sigUbicacion.f == estadoActual.jugador.f && sigUbicacion.c == estadoActual.jugador.c))
			estadoResult.sonambulo = sigUbicacion;
		break;
	case actSON_TURN_SR:
		estadoResult.sonambulo.brujula = (Orientacion)((estadoResult.sonambulo.brujula + 1) % 8);
		break;
	case actSON_TURN_SL:
		estadoResult.sonambulo.brujula = (Orientacion)((estadoResult.sonambulo.brujula + 7) % 8);
		break;
	}
	return estadoResult;
}

bool ComportamientoJugador::casillaTransitable(const ubicacion &pos)
{
	return mapaAux[pos.f][pos.c].esTransitable();
}

list<Action> ComportamientoJugador::busquedaAnchuraJugador(const Estado &origen, const ubicacion &destino) {
	Nodo nodoActual(this);
	list<Nodo> frontera;
	set<Nodo> explorados;
	list<Action> plan;

	bool SolucionEncontrada = (nodoActual.estado.jugador.f == destino.f && nodoActual.estado.jugador.c == destino.c);

	nodoActual.estado = origen;
	frontera.push_back(nodoActual);

	while(!frontera.empty() && !SolucionEncontrada){
		frontera.pop_front();
		explorados.insert(nodoActual);

		// Generar hijo actFORWARD
		Nodo hijoForward = nodoActual;

		hijoForward.estado = aplicar(actFORWARD, nodoActual.estado);

		if(hijoForward.estado.jugador.f == destino.f && hijoForward.estado.jugador.c == destino.c){
			SolucionEncontrada = true;
			nodoActual = hijoForward;
		} else if(explorados.find(hijoForward) == explorados.end()){
			hijoForward.secuencia.push_back(actFORWARD);
			frontera.push_back(hijoForward);
		}

		if(!SolucionEncontrada){
			// Generar hijo actTURN_L
			Nodo hijoTurnL = nodoActual;
			hijoTurnL.estado = aplicar(actTURN_L, nodoActual.estado);

			if(explorados.find(hijoTurnL) == explorados.end()){
				hijoTurnL.secuencia.push_back(actTURN_L);
				frontera.push_back(hijoTurnL);
			}

			// Generar hijo actTURN_R
			Nodo hijoTurnR = nodoActual;
			hijoTurnR.estado = aplicar(actTURN_R, nodoActual.estado);

			if(explorados.find(hijoTurnR) == explorados.end()){
				hijoTurnR.secuencia.push_back(actTURN_R);
				frontera.push_back(hijoTurnR);
			}
		}
	}

	if(SolucionEncontrada){
		plan = nodoActual.secuencia;

		return plan;
	}
	
	// Si no se encuentra un plan, devolvemos una lista vacía
	return list<Action>();
}

// list<Action> ComportamientoJugador::busquedaAnchuraSonambulo(const Estado &origen, const ubicacion &destino)
// {
// 	queue<Nodo *> planBFS;

// 	// COMPLETAR

// 	list<Action> ruta;

// 	// Construye la lista de nodos en el orden correcto
// 	while (!planBFS.empty())
// 	{
// 		Nodo *nodoActual = planBFS.front();
// 		ruta.push_back(nodoActual);
// 		planBFS.pop();
// 	}

// 	return ruta;
// }

// list<Action> ComportamientoJugador::encuentraCaminoDijkstraJugador(const Estado &origen, const ubicacion &destino)
// {
// 	priority_queue<Nodo *> planDijkstra;

// 	// COMPLETAR

// 	list<Action> ruta;

// 	// Construye la lista de nodos en el orden correcto
// 	while (!planDijkstra.empty())
// 	{
// 		Nodo *nodoActual = planDijkstra.top();
// 		ruta.push_front(nodoActual);
// 		planDijkstra.pop();
// 	}

// 	return ruta;
// }

// list<Action> ComportamientoJugador::encuentraCaminoAStarSonambulo(const Estado &origen, const ubicacion &destino)
// {
// 	priority_queue<Nodo *> planAStar;

// 	// COMPLETAR

// 	list<Action> ruta;

// 	// Construye la lista de nodos en el orden correcto
// 	while (!planAStar.empty())
// 	{
// 		Nodo *nodoActual = planAStar.top();
// 		ruta.push_front(nodoActual);
// 		planAStar.pop();
// 	}

// 	return ruta;
// }

// list<Action> ComportamientoJugador::maximizarPuntuacion(const Estado &origen, const ubicacion &destino)
// {
// 	priority_queue<Nodo *> planMaxPuntuacion;

// 	// COMPLETAR

// 	list<Action> ruta;

// 	// Construye la lista de nodos en el orden correcto
// 	while (!planMaxPuntuacion.empty())
// 	{
// 		Nodo *nodoActual = planMaxPuntuacion.top();
// 		ruta.push_front(nodoActual);
// 		planMaxPuntuacion.pop();
// 	}

// 	return ruta;
// }

void ComportamientoJugador::actualizaMapaVisionJugador(const Sensores &sensores)
{
	const int DIM = 4; // dimension de la vision
	int indice = 0;

	int f = estadoActual.jugador.f;
	int c = estadoActual.jugador.c;

	if (mapaResultado[f][c] == '?')
		mapaResultado[f][c] = sensores.terreno[indice];

	switch (estadoActual.jugador.brujula)
	{
	case norte: // vision Norte
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int f = estadoActual.jugador.f - i;
				int c = estadoActual.jugador.c + j;

				if (f >= 0 && f < mapaAux.size() && c >= 0 && c < mapaAux[0].size())
				{
					if (mapaResultado[f][c] == '?')
						mapaResultado[f][c] = sensores.terreno[indice];

					indice++;
				}
			}
		}
		break;
	case noreste: // vision Noreste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int f = estadoActual.jugador.f - i;
				int c = estadoActual.jugador.c + i;

				if (j < 0)
				{
					c += j;
				}
				else if (j > 0)
				{
					f += j;
				}

				if (f >= 0 && f < mapaAux.size() && c >= 0 && c < mapaAux[0].size())
				{
					if (mapaResultado[f][c] == '?')
						mapaResultado[f][c] = sensores.terreno[indice];

					indice++;
				}
			}
		}
		break;
	case este: // vision Este
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int f = estadoActual.jugador.f + j;
				int c = estadoActual.jugador.c + i;

				if (f >= 0 && f < mapaAux.size() && c >= 0 && c < mapaAux[0].size())
				{
					if (mapaResultado[f][c] == '?')
						mapaResultado[f][c] = sensores.terreno[indice];

					indice++;
				}
			}
		}
		break;
	case sureste: // vision Sureste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int f = estadoActual.jugador.f + i;
				int c = estadoActual.jugador.c + i;

				if (j < 0)
				{
					f += j;
				}
				else if (j > 0)
				{
					c -= j;
				}

				if (f >= 0 && f < mapaAux.size() && c >= 0 && c < mapaAux[0].size())
				{
					if (mapaResultado[f][c] == '?')
						mapaResultado[f][c] = sensores.terreno[indice];

					indice++;
				}
			}
		}
		break;
	case sur: // vision Sur
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int f = estadoActual.jugador.f + i;
				int c = estadoActual.jugador.c - j;

				if (f >= 0 && f < mapaAux.size() && c >= 0 && c < mapaAux[0].size())
				{
					if (mapaResultado[f][c] == '?')
						mapaResultado[f][c] = sensores.terreno[indice];

					indice++;
				}
			}
		}
		break;
	case suroeste: // vision Suroeste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int f = estadoActual.jugador.f + i;
				int c = estadoActual.jugador.c - i;

				if (j < 0)
				{
					c -= j;
				}
				else if (j > 0)
				{
					f -= j;
				}

				if (f >= 0 && f < mapaAux.size() && c >= 0 && c < mapaAux[0].size())
				{
					if (mapaResultado[f][c] == '?')
						mapaResultado[f][c] = sensores.terreno[indice];

					indice++;
				}
			}
		}
		break;
	case oeste: // vision Oeste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int f = estadoActual.jugador.f - j;
				int c = estadoActual.jugador.c - i;

				if (f >= 0 && f < mapaAux.size() && c >= 0 && c < mapaAux[0].size())
				{
					if (mapaResultado[f][c] == '?')
						mapaResultado[f][c] = sensores.terreno[indice];

					indice++;
				}
			}
		}
		break;
	case noroeste: // vision Noroeste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int f = estadoActual.jugador.f - i;
				int c = estadoActual.jugador.c - i;

				if (j < 0)
				{
					f -= j;
				}
				else if (j > 0)
				{
					c += j;
				}

				if (f >= 0 && f < mapaAux.size() && c >= 0 && c < mapaAux[0].size())
				{
					if (mapaResultado[f][c] == '?')
						mapaResultado[f][c] = sensores.terreno[indice];

					indice++;
				}
			}
		}
		break;
	}
}

int ComportamientoJugador::calcularCostoBateria(Action accion, unsigned char tipoCasilla)
{
	int costo = 0;

	switch (accion)
	{
	case actFORWARD: // ActFORWARD y actSON_FORWARD
	case actSON_FORWARD:
		switch (tipoCasilla)
		{
		case 'A':
			costo = tieneBikini ? 10 : 100;
			break;
		case 'B':
			costo = tieneZapatillas ? 15 : 50;
			break;
		case 'T':
			costo = 2;
			break;
		default:
			costo = 1;
			break;
		}
		break;
	case actTURN_L: // actTURN_L y actTURN_R
	case actTURN_R:
		switch (tipoCasilla)
		{
		case 'A':
			costo = tieneBikini ? 5 : 25;
			break;
		case 'B':
			costo = tieneZapatillas ? 1 : 5;
			break;
		case 'T':
			costo = 2;
			break;
		default:
			costo = 1;
			break;
		}
		break;
	case actSON_TURN_SL: // actSON_TURN_SL y actSON_TURN_SR
	case actSON_TURN_SR:
		switch (tipoCasilla)
		{
		case 'A':
			costo = tieneBikini ? 2 : 7;
			break;
		case 'B':
			costo = tieneZapatillas ? 1 : 3;
			break;
		case 'T':
			costo = 1;
			break;
		default:
			costo = 1;
			break;
		}
		break;
	case actWHEREIS: // actWHEREIS
		costo = 200;
		break;
	case actIDLE: // actIDLE
		costo = 0;
		break;
	}

	return costo;
}

int ComportamientoJugador::interact(Action accion, int valor)
{
	return false;
}