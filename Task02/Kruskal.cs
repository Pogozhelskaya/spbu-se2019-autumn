
using System.IO;

namespace Task02
{
    public static class Kruskal
    {
        internal class Dsu
        {
            public readonly int[] Parent;
            public readonly int[] Rank;

            public Dsu(int verticesCount)
            {
                Parent = new int[verticesCount];
                Rank = new int[verticesCount];

                for (var i = 0; i < verticesCount; i++)
                {
                    Parent[i] = i;
                    Rank[i] = 0;
                }
            }

            internal int Find(Dsu dsu, int i)
            {
                if (i == dsu.Parent[i])
                {
                    return i;
                }
                dsu.Parent[i] = Find(dsu ,dsu.Parent[i]);
                return Parent[i];
            }

            internal void Union(Dsu dsu, int i, int j)
            {
                i = Find(dsu, i);
                j = Find(dsu, j);

                if (i == j)
                {
                    return;
                }

                if (Rank[i] > Rank[j])
                {
                    Parent[j] = i;
                }
                else
                {
                    Parent[i] = j;
                    if (Rank[i] == Rank[j])
                    {
                        Rank[j]++;
                    }
                }
            }
        }

        private static int ParallelKruskal(this Graph graph)
        {
            var dsu = new Dsu(graph.VerticesCount);
            var edges = graph.EdgesList.ToArray();
            for (int v = 0; v < graph.VerticesCount; v++)
            {
                dsu.Parent[v] = v;
                dsu.Rank[v] = 0;
            }
            
            Sort.QSortParallel(edges, 0 , edges.Length);
            
            int i = 0, cnt = 0, weight = 0;
            
            while (cnt < graph.VerticesCount - 1)
            {
                Graph.Edge nextEdge = edges[i++];

                int x = dsu.Find(dsu, nextEdge.End);
                int y = dsu.Find(dsu, nextEdge.Start);

                if (x != y)
                {
                    edges[cnt++] = nextEdge;
                    dsu.Union(dsu, x, y);
                    weight += nextEdge.Weight;
                }
            }
            return weight;
        }
        public static void PrintKruskal(this Graph graph)
        {
            using var writer = new StreamWriter("Kruskal.txt");
            int result = graph.ParallelKruskal();
            writer.Write($"{result} ");
        }
    }
}