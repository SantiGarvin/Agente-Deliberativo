#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>
#include <queue>
#include <vector>
#include <cmath>
#include <iostream>

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

		Celda(Terreno t)
			: terreno{t},
			  superficie{Entidad::SinEntidad}
		{
		}

		Celda(Terreno t, Entidad s)
			: terreno{t},
			  superficie{s}
		{
		}

		bool esTransitable() const
		{
			return (terreno != Terreno::Precipicio && terreno != Terreno::Muro && superficie != Entidad::Lobo && superficie != Entidad::Aldeano && superficie != Entidad::Sonambulo);
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
			return (jugador == other.jugador && sonambulo == other.sonambulo);
		}
	};

	//////////////////////////////////////////////////////////////////
	// Clase NODO													//
	//////////////////////////////////////////////////////////////////
	struct Nodo
	{
		vector<Action> acciones; // Acciones posibles desde el nodo
		vector<Nodo *> vecinos;	 // Punteros a los nodos vecinos

		Nodo *padre; // Puntero al nodo padre

		ubicacion pos; // Posición del nodo
		Estado estado; // Estado del nodo
		Celda celda;   // Información de la celda
		bool visitado; // Indica si el nodo ha sido visitado

		vector<int> costos; // Costos de las acciones
		int costoAcumulado; // Costo acumulado
		int costoEstimado;	// Costo estimado hasta la meta (para A*)
		int heuristica;		// Heurística para el cálculo del costo estimado

		Nodo()
			: comportamiento{nullptr},
			  acciones{},
			  vecinos{},
			  padre{nullptr},
			  celda{Terreno::Desconocido, Entidad::SinEntidad},
			  pos{-1, -1, Orientacion::norte},
			  estado{},
			  visitado{false},
			  costoAcumulado{0},
			  costoEstimado{0},
			  heuristica{0}
		{
		}

		Nodo(ComportamientoJugador* comp, int f, int c, Orientacion o = Orientacion::norte)
			: comportamiento{comp},
			  acciones{},
			  vecinos{},
			  padre{nullptr},
			  celda{Terreno::Desconocido, Entidad::SinEntidad},
			  pos{f, c, o},
			  visitado{false},
			  costoAcumulado{0},
			  costoEstimado{0},
			  heuristica{0}
		{
		}

		Nodo(ComportamientoJugador* comp, int f, int c, Terreno t = Terreno::Desconocido, Entidad s = Entidad::SinEntidad)
			: comportamiento{comp},
			  acciones{},
			  vecinos{},
			  padre{nullptr},
			  celda{t, s},
			  pos{f, c, Orientacion::norte},
			  visitado{false},
			  costoAcumulado{0},
			  costoEstimado{0},
			  heuristica{0}
		{
		}

		bool operator<(const Nodo &other) const
		{
			int costoJugador = costoAcumulado + heuristica;
			int otherCostoJugador = other.costoAcumulado + other.heuristica;

			int costoSonambulo = costoAcumulado + heuristica;
			int otherCostoSonambulo = other.costoAcumulado + other.heuristica;

			int costoTotal = costoAcumulado + heuristica;
			int otherCostoTotal = other.costoAcumulado + other.heuristica;

			switch (comportamiento->nivel)
			{
			case 0:
				if (estado.jugador.f < other.estado.jugador.f)
					return true;
				else if (estado.jugador.f == other.estado.jugador.f)
					return estado.jugador.c < other.estado.jugador.c;
				else if (estado.jugador.f == other.estado.jugador.f && estado.jugador.c == other.estado.jugador.c)
					return estado.jugador.brujula < other.estado.jugador.brujula;
				else
					return false;
				break;
			case 1:
				if (estado.sonambulo.f < other.estado.sonambulo.f)
					return true;
				else if (estado.sonambulo.f == other.estado.sonambulo.f)
					return estado.sonambulo.c < other.estado.sonambulo.c;
				else if (estado.sonambulo.f == other.estado.sonambulo.f && estado.sonambulo.c == other.estado.sonambulo.c)
					return estado.sonambulo.brujula < other.estado.sonambulo.brujula;
				else
					return false;
				break;
			case 2:
				if (costoJugador < otherCostoJugador)
					return true;
				else if (costoJugador == otherCostoJugador)
					return estado.jugador.c < other.estado.jugador.c;
				else if (costoJugador == otherCostoJugador && estado.jugador.c == other.estado.jugador.c)
					return estado.jugador.brujula < other.estado.jugador.brujula;
				else
					return false;
				break;
			case 3:
				if (costoSonambulo < otherCostoSonambulo)
					return true;
				else if (costoSonambulo == otherCostoSonambulo)
					return estado.sonambulo.c < other.estado.sonambulo.c;
				else if (costoSonambulo == otherCostoSonambulo && estado.sonambulo.c == other.estado.sonambulo.c)
					return estado.sonambulo.brujula < other.estado.sonambulo.brujula;
				else
					return false;
				break;
			case 4:
				if (costoTotal < otherCostoTotal)
					return true;
				else if (costoTotal == otherCostoTotal)
					return (estado.jugador.c + estado.sonambulo.c) < (other.estado.jugador.c + other.estado.sonambulo.c);
				else if (costoTotal == otherCostoTotal && (estado.jugador.c + estado.sonambulo.c) == (other.estado.jugador.c + other.estado.sonambulo.c))
					return (estado.jugador.brujula + estado.sonambulo.brujula) < (other.estado.jugador.brujula + other.estado.sonambulo.brujula);
				else
					return false;
				break;
			}
		}

		bool operator==(const Nodo &other) const
		{
			return (estado == other.estado);
		}

		void conectar(Nodo *otro_nodo, Action accion)
		{
			vecinos.push_back(otro_nodo);
			acciones.push_back(accion);
		}

		void desconectar(Nodo *otro_nodo)
		{
			for (size_t i = 0; i < vecinos.size(); ++i)
			{
				if (vecinos[i] == otro_nodo)
				{
					vecinos.erase(vecinos.begin() + i);
					acciones.erase(acciones.begin() + i);
					break;
				}
			}
		}

		int calcularHeuristica(Nodo *objetivo)
		{
			// Ejemplo de heurística: distancia de Manhattan
			int distancia_f = abs(pos.f - objetivo->pos.f);
			int distancia_c = abs(pos.c - objetivo->pos.c);
			heuristica = distancia_f + distancia_c;
			return heuristica;
		}

		bool esObjetivo(Nodo *objetivo)
		{
			return pos == objetivo->pos;
		}

		bool esTransitable()
		{
			return celda.esTransitable();
		}

		void imprimirInfo()
		{
			std::cout << "Posicion: (" << pos.f << ", " << pos.c << ")" << std::endl;
			std::cout << "Orientacion: " << static_cast<int>(pos.brujula) << std::endl;
			std::cout << "Terreno: " << static_cast<int>(celda.terreno) << std::endl;
			std::cout << "Superficie: " << static_cast<int>(celda.superficie) << std::endl;
			std::cout << "Costo acumulado: " << costoAcumulado << std::endl;
			std::cout << "Heuristica: " << heuristica << std::endl;
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
	bool colision;
	bool reset;

	int tiempo;

	bool tieneBikini;
	bool tieneZapatillas;

	Action ultimaAccionJugador;
	Action ultimaAccionSonambulo;

	//////////////////////////////////////////////////////////////////
	// Planes														//
	//////////////////////////////////////////////////////////////////

	bool hayPlan;

	queue<Action> planBFS;
	priority_queue<Action> planAStar;
	priority_queue<Action> planDijkstra;

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

	vector<unsigned char> vision;

	//////////////////////////////////////////////////////////////////
	// Funciones privadas											//
	//////////////////////////////////////////////////////////////////

	// INICIALIZACION
	void inicializaVariablesEstado();
	void inicializarGrafo(int tamanio);

	// ACTUALIZACIONES
	void actualizaEstado(const Sensores &sensores);
	void actualizaPosicionOrientacion(bool esJugador);
	void actualizaGrafo(const Mapa &mapa);
	void actualizaMapaAux();
	void actualizaMapaVisionJugador(const Sensores &sensores);

	// FUNCIONES AUXILIARES
	int calcularCostoBateria(Action accion, unsigned char tipoCasilla);
	list<Action> busquedaAnchuraJugador();
	// list<Action> busquedaAnchuraSonambulo();
	// list<Action> encuentraCaminoDijkstraJugador();
	// list<Action> encuentraCaminoAStarSonambulo();
	// list<Action> maximizarPuntuacion();

	ubicacion siguienteCasilla(const ubicacion &pos);

	void conectarNodos(int fila1, int columna1, int fila2, int columna2);

	// Action eligeAccionPlan();

	Action accionEntreNodos(Nodo *origen, Nodo *destino);

	bool esObjetivo(const Nodo &objetivo);
	Orientacion orientacionEntreNodos(const Nodo &origen, const Nodo &destino);

	void anularMatriz(vector<vector<unsigned char>> &matriz);
	void visualizaPlan(const list<Action> &plan);

	ubicacion nextCasilla(const ubicacion &pos);

	// DEBUG
	char orientacionASimbolo(Orientacion o);
	void imprimirGrafo();

	Terreno charToTerreno(unsigned char c);
	Entidad charToEntidad(unsigned char c);
};

#endif
