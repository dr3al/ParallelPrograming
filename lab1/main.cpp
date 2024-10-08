#include "clusterSystem.h"

int main() {
    Cluster cluster;

    // Создание узлов вручную с использованием конструктора
    ClusterNode node1(CpuSpec("Intel_i9", 8, 3.6), GpuSpec("NVIDIA_RTX_3080", 10240), RamSpec("Kingston", 32768), LanSpec("Ethernet", 1.0));
    ClusterNode node2(CpuSpec("AMD_Ryzen_7", 12, 3.8), GpuSpec("NVIDIA_RTX_3070", 8192), RamSpec("Kingston", 16384), LanSpec("WiFi", 0.5));

    cluster.AddNode(node1);
    cluster.AddNode(node2);

    // Экспорт данных
    cluster.Export("out.txt");

    // Импорт данных
    Cluster importedCluster;
    importedCluster.Import("in.txt");

    // Вывод данных
    importedCluster.Print();

    return 0;
}
