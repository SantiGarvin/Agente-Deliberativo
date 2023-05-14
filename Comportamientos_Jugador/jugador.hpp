#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>
#include <set>
#include <unordered_map>
#include <queue>
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <memory>
#include <limits>
#include <sstream>

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

	struct Objeto
	{
		bool zapatillas;
		bool bikini;

		Objeto() : zapatillas(false), bikini(false) {}

		bool operator==(const Objeto &o) const
		{
			return zapatillas == o.zapatillas && bikini == o.bikini;
		}
	};

	struct Estado
	{
		ubicacion jugador;
		ubicacion sonambulo;

		Objeto objetoJugador;
		Objeto objetoSonambulo;

		Estado()
			: comportamiento{nullptr},
			  jugador{-1, -1, Orientacion::norte},
			  sonambulo{-1, -1, Orientacion::norte}
		{
		}

		Estado(ComportamientoJugador *c)
			: comportamiento{c},
			  jugador{-1, -1, Orientacion::norte},
			  sonambulo{-1, -1, Orientacion::norte}
		{
		}

		Estado(ComportamientoJugador *c, ubicacion uj, ubicacion us)
			: comportamiento{c},
			  jugador{uj},
			  sonambulo{us}
		{
		}

		bool operator==(const Estado &other) const
		{
			switch (comportamiento->nivel)
			{
			case 0:
				return jugador == other.jugador;
			case 1:
				return jugador == other.jugador && sonambulo.f == other.sonambulo.f && sonambulo.c == other.sonambulo.c;
			case 2:
				return jugador == other.jugador && sonambulo.f == other.sonambulo.f && sonambulo.c == other.sonambulo.c && objetoJugador == other.objetoJugador;
			}
		}

	private:
		ComportamientoJugador *comportamiento;
	};

	struct EstadoHasherJugador
	{
		size_t operator()(const Estado &e) const
		{
			return (hash<int>()(e.jugador.f) ^ (hash<int>()(e.jugador.c) << 1)) ^ (hash<int>()(e.jugador.brujula) << 2) ^ (hash<int>()(e.objetoJugador.zapatillas) << 3) ^ (hash<int>()(e.objetoJugador.bikini) << 4);
		}

		size_t operator()(const std::string &clave) const
		{
			std::hash<std::string> hasher;
			return hasher(clave);
		}
	};

	struct EstadoHasherSonambulo
	{
		size_t operator()(const Estado &e) const
		{
			return (hash<int>()(e.sonambulo.f) ^ (hash<int>()(e.sonambulo.c) << 1)) ^ (hash<int>()(e.sonambulo.brujula) << 2) ^ (hash<int>()(e.objetoSonambulo.zapatillas) << 3) ^ (hash<int>()(e.objetoSonambulo.bikini) << 4);
		}

		size_t operator()(const std::string &clave) const
		{
			std::hash<std::string> hasher;
			return hasher(clave);
		};
	};

	//////////////////////////////////////////////////////////////////
	// Struct NODO													//
	//////////////////////////////////////////////////////////////////
	struct Nodo
	{
		Action accion; // DEBUG: Acción que se ha realizado para llegar al nodo
		Estado estado; // Estado del nodo
		Celda celda;   // Información de la celda

		list<Action> secuencia; // Acciones que se han realizado para llegar al nodo

		bool valido; // Indica si el nodo se encuentra en un estado válido

		int costoAcumulado; // Costo acumulado
		int costoEstimado;	// Costo estimado hasta la meta (para A*)

		Nodo()
			: comportamiento{nullptr},
			  accion{Action::actIDLE},
			  celda{Terreno::Desconocido, Entidad::SinEntidad},
			  estado{},
			  valido{true},
			  secuencia{},
			  costoAcumulado{0},
			  costoEstimado{0}
		{
		}

		Nodo(ComportamientoJugador *c)
			: comportamiento{c},
			  accion{Action::actIDLE},
			  celda{Terreno::Desconocido, Entidad::SinEntidad},
			  estado{c},
			  valido{true},
			  secuencia{},
			  costoAcumulado{0},
			  costoEstimado{0}
		{
		}

		Nodo(ComportamientoJugador *c, Action a, const Estado &e, const Celda &cd, const list<Action> &seq, int costoAcum, int costoEst)
			: comportamiento{c},
			  accion{a},
			  estado{e},
			  valido{true},
			  celda{cd},
			  secuencia{seq},
			  costoAcumulado{costoAcum},
			  costoEstimado{costoEst}
		{
		}

		// bool operator>(const Nodo &other) const
		// {
		// 	int costoThis = costoAcumulado + costoEstimado;
		// 	int costoOther = other.costoAcumulado + other.costoEstimado;

		// 	return costoThis < costoOther;
		// }

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
			case 2:
				return this->costoAcumulado > other.costoAcumulado;
				break;
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

	ComportamientoJugador(unsigned int size)
		: Comportamiento(size),
		  grafo(size, vector<Nodo>(size)),
		  mapaAux(2 * size, vector<Celda>(2 * size)),
		  mapaCeldas(size, vector<Celda>(size)),
		  estadoActual(this)
	{
		inicializaVariablesEstado();
	}

	ComportamientoJugador(std::vector<std::vector<unsigned char>> mapaR)
		: Comportamiento(mapaR),
		  grafo(mapaR.size(), vector<Nodo>(mapaR.size())),
		  mapaAux(2 * mapaR.size(), vector<Celda>(2 * mapaR.size())),
		  mapaCeldas(mapaR.size(), vector<Celda>(mapaR.size())),
		  estadoActual(this)
	{
		inicializaVariablesEstado();
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

	list<Action> plan;

	//////////////////////////////////////////////////////////////////
	// Mapas - Ubicaciones											//
	//////////////////////////////////////////////////////////////////

	Mapa mapaCeldas;
	Mapa mapaAux; // solo para nivel 4
	Grafo grafo;

	vector<Celda> vision;
	// vector<vector<Celda>> areaLocal;

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
	void actualizarMapaCeldas();

	// FUNCIONES AUXILIARES
	list<Action> busquedaAnchuraJugador(const Estado &origen, const ubicacion &destino);
	list<Action> busquedaAnchuraSonambulo(const Estado &origen, const ubicacion &destino);
	list<Action> busquedaDijkstraJugador(const Estado &origen, const ubicacion &destino);
	// list<Action> encuentraCaminoAStarSonambulo(const Estado &origen, const ubicacion &destino);
	// list<Action> maximizarPuntuacion(const Estado &origen, const ubicacion &destino);

	int calcularCostoBateria(Action accion, unsigned char tipoCasilla);

	ubicacion siguienteCasilla(const ubicacion &pos);

	void anularMapaConPlan();
	void visualizaPlan(const list<Action> &plan);

	Terreno charATerreno(unsigned char c) const;
	Entidad charAEntidad(unsigned char c) const;

	Estado aplicar(const Action &a, const Estado &est);

	bool casillaTransitable(const ubicacion &pos);

	bool esDestinoJugador(const Estado &nodo, const ubicacion &destino);
	bool esDestinoSonambulo(const Estado &nodo, const ubicacion &destino);

	const vector<vector<Celda>> &getMapaCeldas() const;
	vector<vector<Celda>> &getMapaCeldas();

	vector<vector<Celda>> getAreaLocalJugador(const vector<vector<Celda>> &mapa, const Estado &estado);
	vector<vector<Celda>> getAreaLocalSonambulo(const vector<vector<Celda>> &mapa, const Estado &estado);

	void procesarAccion(const Nodo *n, const Action &a, list<Nodo> &frontera, set<Nodo> &explorados);

	// Action mejorMovimiento(const Estado &estado, const vector<vector<Celda>> &mapa, Entidad agente);

	Nodo aplicarAccionCosto(const Nodo &nodo, const Action &accion);

	string generarClave(const Estado &estado);

	int calcularCostoBateria(Action accion, unsigned char tipoCasilla, const Objeto &objetoJugador);
	//////////////////////////////////////////////////////////////////
	void debug(bool imprimir) const;
	string toString(Orientacion orientacion) const;
	string toString(Action accion) const;
	//////////////////////////////////////////////////////////////////
};

#endif
