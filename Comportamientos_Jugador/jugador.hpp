#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>
#include <set>
#include <queue>
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <memory>

using namespace std;

class ComportamientoJugador : public Comportamiento
{
public:
	struct Nodo;
	typedef vector<vector<Nodo>> Grafo;

	struct Celda;
	typedef vector<vector<Celda>> Mapa;

	enum Terreno
	{
		Bosque = 'B',
		Agua = 'A',
		Precipicio = 'P',
		SueloPiedra = 'S',
		SueloArenoso = 'T',
		Muro = 'M',
		Bikini = 'K',
		Zapatillas = 'D',
		Recarga = 'X',
		Desconocido = '?'
	};

	enum Entidad
	{
		Jugador = 'j',
		Sonambulo = 's',
		Aldeano = 'a',
		Lobo = 'l',
		SinEntidad = '_'
	};

	struct Celda
	{
		unsigned char terreno;
		unsigned char superficie;

		Celda()
			: terreno{Terreno::Desconocido},
			  superficie{Entidad::SinEntidad}
		{
		}

		Celda(Terreno t, Entidad s = Entidad::SinEntidad)
			: terreno{t},
			  superficie{s}
		{
		}

		bool esTransitable() const
		{
			return (terreno != Terreno::Precipicio && terreno != Terreno::Muro && superficie != Entidad::Lobo && superficie != Entidad::Aldeano);
		}
	};

	struct Estado
	{
		ubicacion jugador;
		ubicacion sonambulo;

		Estado()
		{
			jugador = {-1, -1, Orientacion::norte};
			sonambulo = {-1, -1, Orientacion::norte};
		}

		Estado(ubicacion uj, ubicacion us)
		{
			jugador = uj;
			sonambulo = us;
		}

		bool operator==(const Estado &other) const
		{
			return (jugador == other.jugador && sonambulo.f == other.sonambulo.f && sonambulo.c == other.sonambulo.c);
		}
	};

	//////////////////////////////////////////////////////////////////
	// Struct NODO													//
	//////////////////////////////////////////////////////////////////
	struct Nodo
	{
		Action accion;
		Estado estado; // Estado del nodo
		Celda celda;   // Información de la celda

		list<Action> secuencia;				 // Acciones que se han realizado para llegar al nodo

		int costoAcumulado; // Costo acumulado
		int costoEstimado;	// Costo estimado hasta la meta (para A*)

		Nodo()
			: comportamiento{nullptr},
			//   padre{nullptr},
			  celda{Terreno::Desconocido, Entidad::SinEntidad},
			  estado{},
			//   acciones{std::make_shared<std::vector<Action>>()},
			  costoAcumulado{0},
			  costoEstimado{0}
		{
		}

		Nodo(ComportamientoJugador *c)
			: comportamiento{c},
			//   padre{nullptr},
			  celda{Terreno::Desconocido, Entidad::SinEntidad},
			  estado{},
			//   acciones{std::make_shared<std::vector<Action>>()},
			  costoAcumulado{0},
			  costoEstimado{0}
		{
		}

		// bool operator<(const Nodo &other) const
		// {
		// 	int costoThis = costoAcumulado + heuristica;
		// 	int costoOther = other.costoAcumulado + other.heuristica;

		// 	return costoThis > costoOther;
		// }

		//----------------------------------------------------------

		bool operator<(const Nodo &other) const
		{
			// int costoJugador = costoAcumulado + heuristica;
			// int otherCostoJugador = other.costoAcumulado + other.heuristica;

			// int costoSonambulo = costoAcumulado + heuristica;
			// int otherCostoSonambulo = other.costoAcumulado + other.heuristica;

			// int costoTotal = costoAcumulado + heuristica;
			// int otherCostoTotal = other.costoAcumulado + other.heuristica;

			switch (comportamiento->nivel)
			{
			case 0:
				if (estado.jugador.f < other.estado.jugador.f)
					return true;
				else if (estado.jugador.f == other.estado.jugador.f && estado.jugador.c < other.estado.jugador.c)
					return true;
				else if (estado.jugador.f == other.estado.jugador.f && estado.jugador.c == other.estado.jugador.c && estado.jugador.brujula < other.estado.jugador.brujula)
					return true;
				else
					return false;
				break;

			case 1:
				// if (estado.jugador.f < other.estado.jugador.f || estado.sonambulo.f < other.estado.sonambulo.f)
				// 	return true;
				// else if ((estado.jugador.f == other.estado.jugador.f || estado.sonambulo.f == other.estado.sonambulo.f) && (estado.jugador.c < other.estado.jugador.c || estado.sonambulo.c < other.estado.sonambulo.c))
				// 	return true;
				// else if ((estado.jugador.f == other.estado.jugador.f || estado.sonambulo.f == other.estado.sonambulo.f) && (estado.jugador.c == other.estado.jugador.c || estado.sonambulo.c == other.estado.sonambulo.c) && (estado.jugador.brujula < other.estado.jugador.brujula || estado.sonambulo.brujula < other.estado.sonambulo.brujula))
				// 	return true;
				// else
				// 	return false;
				// break;
				if (estado.jugador.f < other.estado.jugador.f)
					return true;
				else if (estado.jugador.f == other.estado.jugador.f && estado.jugador.c < other.estado.jugador.c)
					return true;
				else if (estado.jugador.f == other.estado.jugador.f && estado.jugador.c == other.estado.jugador.c && estado.jugador.brujula < other.estado.jugador.brujula)
					return true;
				else if (estado.jugador.f == other.estado.jugador.f && estado.jugador.c == other.estado.jugador.c && estado.jugador.brujula == other.estado.jugador.brujula && estado.sonambulo.f < other.estado.sonambulo.f)
					return true;
				else if (estado.jugador.f == other.estado.jugador.f && estado.jugador.c == other.estado.jugador.c && estado.jugador.brujula == other.estado.jugador.brujula && estado.sonambulo.f == other.estado.sonambulo.f && estado.sonambulo.c < other.estado.sonambulo.c)
					return true;
				else if (estado.jugador.f == other.estado.jugador.f && estado.jugador.c == other.estado.jugador.c && estado.jugador.brujula == other.estado.jugador.brujula && estado.sonambulo.f == other.estado.sonambulo.f && estado.sonambulo.c == other.estado.sonambulo.c && estado.sonambulo.brujula < other.estado.sonambulo.brujula)
					return true;
				else
					return false;
				break;
				// case 2:
				// 	if (costoJugador < otherCostoJugador)
				// 		return true;
				// 	else if (costoJugador == otherCostoJugador)
				// 		return estado.jugador.c < other.estado.jugador.c;
				// 	else if (costoJugador == otherCostoJugador && estado.jugador.c == other.estado.jugador.c)
				// 		return estado.jugador.brujula < other.estado.jugador.brujula;
				// 	else
				// 		return false;
				// 	break;
				// case 3:
				// 	if (costoSonambulo < otherCostoSonambulo)
				// 		return true;
				// 	else if (costoSonambulo == otherCostoSonambulo)
				// 		return estado.sonambulo.c < other.estado.sonambulo.c;
				// 	else if (costoSonambulo == otherCostoSonambulo && estado.sonambulo.c == other.estado.sonambulo.c)
				// 		return estado.sonambulo.brujula < other.estado.sonambulo.brujula;
				// 	else
				// 		return false;
				// 	break;
				// case 4:
				// 	if (costoTotal < otherCostoTotal)
				// 		return true;
				// 	else if (costoTotal == otherCostoTotal)
				// 		return (estado.jugador.c + estado.sonambulo.c) < (other.estado.jugador.c + other.estado.sonambulo.c);
				// 	else if (costoTotal == otherCostoTotal && (estado.jugador.c + estado.sonambulo.c) == (other.estado.jugador.c + other.estado.sonambulo.c))
				// 		return (estado.jugador.brujula + estado.sonambulo.brujula) < (other.estado.jugador.brujula + other.estado.sonambulo.brujula);
				// 	else
				// 		return false;
				// 	break;
			}
			return false;
		}

		//----------------------------------------------------------

		bool operator==(const Nodo &other) const
		{
			return (estado == other.estado);
		}

		// int calcularHeuristica(Nodo *objetivo)
		// {
		// 	// Ejemplo de heurística: distancia de Manhattan
		// 	int distancia_f = abs(pos.f - objetivo->pos.f);
		// 	int distancia_c = abs(pos.c - objetivo->pos.c);
		// 	heuristica = distancia_f + distancia_c;
		// 	return heuristica;
		// }

		bool esTransitable()
		{
			return celda.esTransitable();
		}

	private:
		ComportamientoJugador *comportamiento;
	};

	////////////////////////////////////////////////////////////////

	ComportamientoJugador(unsigned int size) : Comportamiento(size), grafo(size, vector<Nodo>(size)), mapaAux(2 * size, vector<Celda>(2 * size))
	{
		inicializaVariablesEstado();
		dimensionMapa = size;
	}

	ComportamientoJugador(std::vector<std::vector<unsigned char>> mapaR) : Comportamiento(mapaR), grafo(mapaR.size(), vector<Nodo>(mapaR.size())), mapaAux(2 * mapaR.size(), vector<Celda>(2 * mapaR.size()))
	{
		inicializaVariablesEstado();
		dimensionMapa = mapaR.size();
	}

	ComportamientoJugador(const ComportamientoJugador &comport) : Comportamiento(comport) {}
	~ComportamientoJugador() {}

	Action think(Sensores sensores);
	int interact(Action accion, int valor);

private:
	//////////////////////////////////////////////////////////////////
	// Variables de estado											//
	//////////////////////////////////////////////////////////////////

	Estado estadoActual;

	int nivel;
	int bateria;
	bool primeraIteracion;
	bool sonambuloEnVision;

	int tiempo;

	bool tieneBikini;
	bool tieneZapatillas;

	Action ultimaAccionJugador;
	Action ultimaAccionSonambulo;

	bool hayPlan;

	//////////////////////////////////////////////////////////////////
	// Mapas - Ubicaciones											//
	//////////////////////////////////////////////////////////////////

	const int MAX_MAPA = 100;
	int dimensionMapa;

	Mapa mapaAux; // solo para nivel 4
	Grafo grafo;
	list<Action> plan;

	vector<ubicacion> sonambulos;
	vector<ubicacion> aldeanos;
	vector<ubicacion> lobos;

	vector<Celda> vision;

	//////////////////////////////////////////////////////////////////
	// Funciones privadas											//
	//////////////////////////////////////////////////////////////////

	// INICIALIZACION
	void inicializaVariablesEstado();

	// ACTUALIZACIONES
	void actualizaEstado(const Sensores &sensores);
	void actualizaPosicionOrientacion();
	void actualizaGrafo(const Mapa &mapa);
	void actualizaMapaAux();
	void actualizaVisionJugador(const Estado &estado);
	void actualizaMapaResultVisionJugador(const Sensores &sensores);

	// FUNCIONES AUXILIARES
	int calcularCostoBateria(Action accion, unsigned char tipoCasilla);
	list<Action> busquedaAnchuraJugador(const Estado &origen, const ubicacion &destino);
	list<Action> busquedaAnchuraSonambulo(const Estado &origen, const ubicacion &destino);
	// list<Action> encuentraCaminoDijkstraJugador(const Estado &origen, const ubicacion &destino);
	// list<Action> encuentraCaminoAStarSonambulo(const Estado &origen, const ubicacion &destino);
	// list<Action> maximizarPuntuacion(const Estado &origen, const ubicacion &destino);

	ubicacion siguienteCasilla(const ubicacion &pos);

	void anularMapaConPlan();
	void visualizaPlan(const list<Action> &plan);

	Terreno charToTerreno(unsigned char c);
	Entidad charToEntidad(unsigned char c);

	Estado aplicar(const Action &a, const Estado &est);

	bool casillaTransitable(const ubicacion &pos);

	bool esDestinoJ(const Estado &nodo, const ubicacion &destino);
	bool esDestinoS(const Estado &nodo, const ubicacion &destino);

	// list<Action> reconstruirCamino(const Nodo *nodoObjetivo);

	void procesarAccion(const Nodo *n, const Action &a, list<Nodo> &frontera, set<Nodo> &explorados);

	//////////////////////////////////////////////////////////////////
	void debug(bool imprimir) const;
	string toString(Orientacion orientacion) const;
	string toString(Action accion) const;
	//////////////////////////////////////////////////////////////////
};

#endif
