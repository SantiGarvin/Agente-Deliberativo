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

// Este es el método principal que se piden en la practica.
// Tiene como entrada la información de los sensores y devuelve la acción a realizar.
// Para ver los distintos sensores mirar fichero "comportamiento.hpp"
Action ComportamientoJugador::think(Sensores sensores)
{
	Action accion = actIDLE;

	// Actualizar información de los sensores
	actualizaEstado(sensores);

	if (sensores.nivel == 4){
		actualizaMapaVisionJugador(sensores);
		actualizaGrafo(mapaAux);
	} else {
		actualizaMapaAux();
		actualizaGrafo(mapaAux);
	}

	if (ruta.empty())
	{
		// Lógica del agente
		switch (estadoActual.nivel)
		{
		case 0:
			ruta = busquedaAnchuraJugador();
			break;
		case 1:
			ruta = busquedaAnchuraSonambulo();
			break;
		case 2:
			ruta = encuentraCaminoDijkstraJugador();
			break;
		case 3:
			ruta = encuentraCaminoAStarSonambulo();
			break;
		case 4:
			ruta = maximizarPuntuacion();
			break;
		}
		hayPlan = true;
	}
	if (!ruta.empty())
	{
		// completar
	}
	else
	{
		accion = actIDLE;
	}


	//////////////////////////////////////////////////////////////////
	// DEBUG														//
	//////////////////////////////////////////////////////////////////
	bool debug = false;
	if(debug){
		int f = estadoActual.jugador.f;
		int c = estadoActual.jugador.c;

		grafo[f][c].imprimirInfo();
		cout << endl
			<< endl;
		imprimirGrafo();
	}
	//////////////////////////////////////////////////////////////////

	return accion;
}

void ComportamientoJugador::actualizaEstado(const Sensores &sensores)
{
	estadoActual.nivel = sensores.nivel;

	if (estadoActual.nivel != 4)
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

	if (mapaAux[f][c].terreno == Terreno::Bikini && !estadoActual.tieneBikini && !estadoActual.tieneZapatillas)
		estadoActual.tieneBikini = true;
	else
		estadoActual.tieneBikini = false;

	if (mapaAux[f][c].terreno == Terreno::Zapatillas && !estadoActual.tieneZapatillas && !estadoActual.tieneBikini)
		estadoActual.tieneZapatillas = true;
	else
		estadoActual.tieneZapatillas = false;

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

void ComportamientoJugador::actualizaMapaAux(){
	for(int i=0; i < mapaAux.size(); ++i){
		for(int j=0; j < mapaAux[0].size(); ++j){
			mapaAux[i][j].terreno = charToTerreno(mapaResultado[i][j]);
			mapaAux[i][j].superficie = charToEntidad(mapaEntidades[i][j]);
		}
	}
}

void ComportamientoJugador::actualizaGrafo(const Mapa &mapa)
{
    int filas = mapa.size();
    int columnas = mapa[0].size();

    // Coordenadas de desplazamiento: arriba, derecha, abajo, izquierda, y las diagonales.
    int dfil[] = {-1, 0, 1, 0, -1, 1, 1, -1};
    int dcol[] = {0, 1, 0, -1, 1, 1, -1, -1};

    for (int f = 0; f < filas; ++f)
    {
        for (int c = 0; c < columnas; ++c)
        {
			grafo[f][c].celda.terreno = mapaResultado[f][c];
			grafo[f][c].celda.superficie = mapaEntidades[f][c];
            // Limpiar todos los vecinos del nodo actual para luego agregar solo los válidos.
            grafo[f][c].vecinos.clear();

            // Comprobar cada vecino en las 8 direcciones posibles.
            for (int i = 0; i < 8; ++i)
            {
                int fVecino = f + dfil[i];
                int cVecino = c + dcol[i];

                // Si el vecino está dentro de los límites del mapa y es transitable.
                if (fVecino >= 0 && fVecino < filas && cVecino >= 0 && cVecino < columnas &&
                    (mapa[fVecino][cVecino]).esTransitable())
                {
                    // Añadir el vecino al vector de vecinos del nodo actual.
                    grafo[f][c].vecinos.push_back(&grafo[fVecino][cVecino]);
                }
            }
        }
    }
}

Terreno ComportamientoJugador::charToTerreno(unsigned char c) {
    switch (c) {
        case 'B': return Terreno::Bosque;
        case 'A': return Terreno::Agua;
        case 'P': return Terreno::Precipicio;
        case 'S': return Terreno::SueloPiedra;
        case 'T': return Terreno::SueloArenoso;
        case 'M': return Terreno::Muro;
        case 'K': return Terreno::Bikini;
        case 'D': return Terreno::Zapatillas;
        case 'X': return Terreno::Recarga;
        default: return Terreno::Desconocido;
    }
}

Entidad ComportamientoJugador::charToEntidad(unsigned char c) {
    switch (c) {
        case 'j': return Entidad::Jugador;
        case 's': return Entidad::Sonambulo;
        case 'a': return Entidad::Aldeano;
        case 'l': return Entidad::Lobo;
        default: return Entidad::SinEntidad;
    }
}


Orientacion ComportamientoJugador::orientacionEntreNodos(const Nodo &origen, const Nodo &destino){
    int dFil = destino.pos.f - origen.pos.f;
    int dCol = destino.pos.c - origen.pos.c;

    if (dFil == -1 && dCol == 0) {
        return Orientacion::norte;
    } else if (dFil == -1 && dCol == 1) {
        return Orientacion::noreste;
    } else if (dFil == 0 && dCol == 1) {
        return Orientacion::este;
    } else if (dFil == 1 && dCol == 1) {
        return Orientacion::sureste;
    } else if (dFil == 1 && dCol == 0) {
        return Orientacion::sur;
    } else if (dFil == 1 && dCol == -1) {
        return Orientacion::suroeste;
    } else if (dFil == 0 && dCol == -1) {
        return Orientacion::oeste;
    } else if (dFil == -1 && dCol == -1) {
        return Orientacion::noroeste;
    } else {
        // En caso de que los nodos sean iguales o no sean adyacentes, devuelve una orientación por defecto
        return Orientacion::norte;
    }
}

void ComportamientoJugador::inicializaVariablesEstado()
{
	estadoActual.nivel = 0;
	estadoActual.colision = false;
	estadoActual.reset = false;
	estadoActual.jugador.f = 99;
	estadoActual.jugador.c = 99;
	estadoActual.jugador.brujula = norte;
	estadoActual.sonambulo.f = 99;
	estadoActual.sonambulo.c = 99;
	estadoActual.sonambulo.brujula = norte;
	estadoActual.tieneBikini = false;
	estadoActual.tieneZapatillas = false;

	tiempo = 0;
	hayPlan = false;

	ultimaAccionJugador = actIDLE;
	ultimaAccionSonambulo = actIDLE;
}

void ComportamientoJugador::inicializarGrafo(int tamanio)
{
	for (int i = 0; i < tamanio; ++i)
	{
		for (int j = 0; j < tamanio; ++j)
		{
			grafo[i][j] = Nodo(i, j, Orientacion::norte);
		}
	}
}

void ComportamientoJugador::anularMatriz(vector<vector<unsigned char>> &matriz){
	for (int i = 0; i < matriz.size(); ++i) {
		for (int j = 0; j < matriz.size(); ++j) {
			matriz[i][j] = 0;
		}
	}
}

ubicacion ComportamientoJugador::nextCasilla(const ubicacion &pos){
	ubicacion next = pos;

	switch(pos.brujula){
		case norte:
			next.f = pos.f -1;
			break;
		case noreste:
			next.f = pos.f -1;
			next.c = pos.c +1;
			break;
		case este:
			next.c = pos.c +1;
			break;
		case sureste:
			next.f = pos.f +1;
			next.c = pos.c +1;
			break;
		case sur:
			next.f = pos.f +1;
			break;
		case suroeste:
			next.f = pos.f +1;
			next.c = pos.c -1;
			break;
		case oeste:
			next.c = pos.c -1;
			break;
		case noroeste:
			next.f = pos.f -1;
			next.c = pos.c -1;
			break;
		default:
			break;
	}
}

void ComportamientoJugador::visualizaPlan(const list<Action> &plan)
{
	// anularMatriz(mapaConPlan);
	Estado cst = estadoActual;

	auto it = plan.begin();
	while(it != plan.end()){
		switch(*it){
			case actFORWARD:
				cst.jugador = nextCasilla(cst.jugador);
				mapaConPlan[cst.jugador.f][cst.jugador.c] = 1;
			case actTURN_R:
				cst.jugador.brujula = (Orientacion)((cst.jugador.brujula + 2) % 8);
				break;
			case actTURN_L:
				cst.jugador.brujula = (Orientacion)((cst.jugador.brujula + 6) % 8);
				break;
			case actSON_FORWARD:
				cst.sonambulo = nextCasilla(cst.sonambulo);
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

char ComportamientoJugador::orientacionASimbolo(Orientacion o)
{
	switch (o)
	{
	case norte:
		return '^';
	case noreste:
		return '/';
	case este:
		return '>';
	case sureste:
		return '\\';
	case sur:
		return 'v';
	case suroeste:
		return '/';
	case oeste:
		return '<';
	case noroeste:
		return '\\';
	default:
		return '?';
	}
}

void ComportamientoJugador::conectarNodos(int fila1, int columna1, int fila2, int columna2)
{
	// Conectar los nodos (fila1, columna1) y (fila2, columna2)
	Nodo &nodo1 = grafo[fila1][columna1];
	Nodo &nodo2 = grafo[fila2][columna2];
	nodo1.vecinos.push_back(&nodo2);
	nodo2.vecinos.push_back(&nodo1);
}

void ComportamientoJugador::imprimirGrafo()
{
	for (const auto &fila : grafo)
	{
		for (const auto &nodo : fila)
		{
			char simbolo = orientacionASimbolo(nodo.pos.brujula);
			cout << simbolo << ' ';
		}
		cout << '\n';
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

Action ComportamientoJugador::accionEntreNodos(Nodo *nodoOrigen, Nodo *nodoDestino)
{
    for (size_t i = 0; i < nodoOrigen->vecinos.size(); ++i)
    {
        if (nodoOrigen->vecinos[i] == nodoDestino)
        {
            return nodoOrigen->acciones[i];
        }
    }

    // Si no se encuentra una acción entre los nodos, retorna actIDLE
    return actIDLE;
}

list<Action> ComportamientoJugador::busquedaAnchuraJugador()
{
	queue<Nodo *> planBFS;

	// COMPLETAR

	list<Action> ruta;

	// Construye la lista de nodos en el orden correcto
	while (!planBFS.empty())
	{
		Nodo *nodoActual = planBFS.front();
		ruta.push_back(nodoActual);
		planBFS.pop();
	}

	return ruta;
}

list<Action> ComportamientoJugador::busquedaAnchuraSonambulo()
{
	queue<Nodo *> planBFS;

	// COMPLETAR

	list<Action> ruta;

	// Construye la lista de nodos en el orden correcto
	while (!planBFS.empty())
	{
		Nodo *nodoActual = planBFS.front();
		ruta.push_back(nodoActual);
		planBFS.pop();
	}

	return ruta;
}

list<Action> ComportamientoJugador::encuentraCaminoDijkstraJugador()
{
	priority_queue<Nodo *> planDijkstra;

	// COMPLETAR

	list<Action> ruta;

	// Construye la lista de nodos en el orden correcto
	while (!planDijkstra.empty())
	{
		Nodo *nodoActual = planDijkstra.top();
		ruta.push_front(nodoActual);
		planDijkstra.pop();
	}

	return ruta;
}

list<Action> ComportamientoJugador::encuentraCaminoAStarSonambulo()
{
	priority_queue<Nodo *> planAStar;

	// COMPLETAR

	list<Action> ruta;

	// Construye la lista de nodos en el orden correcto
	while (!planAStar.empty())
	{
		Nodo *nodoActual = planAStar.top();
		ruta.push_front(nodoActual);
		planAStar.pop();
	}

	return ruta;
}

list<Action> ComportamientoJugador::maximizarPuntuacion()
{
	priority_queue<Nodo *> planMaxPuntuacion;

	// COMPLETAR

	list<Action> ruta;

	// Construye la lista de nodos en el orden correcto
	while (!planMaxPuntuacion.empty())
	{
		Nodo *nodoActual = planMaxPuntuacion.top();
		ruta.push_front(nodoActual);
		planMaxPuntuacion.pop();
	}

	return ruta;
}

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

bool ComportamientoJugador::esObjetivo(const Nodo &objetivo){
	return estadoActual.jugador.f == objetivo.pos.f && estadoActual.jugador.c == objetivo.pos.c;
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
			costo = estadoActual.tieneBikini ? 10 : 100;
			break;
		case 'B':
			costo = estadoActual.tieneZapatillas ? 15 : 50;
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
			costo = estadoActual.tieneBikini ? 5 : 25;
			break;
		case 'B':
			costo = estadoActual.tieneZapatillas ? 1 : 5;
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
			costo = estadoActual.tieneBikini ? 2 : 7;
			break;
		case 'B':
			costo = estadoActual.tieneZapatillas ? 1 : 3;
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