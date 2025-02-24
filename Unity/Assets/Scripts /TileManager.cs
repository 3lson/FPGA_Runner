using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

public class TileManager : MonoBehaviour
{
    public GameObject[] tilePrefabs;
    public float zSpawn = 0;
    public float tileLength = 30;
    public int numberofTiles = 3;
    private List<GameObject> activeTiles = new List<GameObject>();
    public Transform playerTransform;
    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void Start()
    {
        for (int i=0;i<numberofTiles; i++)
        {
            if (i==0){
                SpawnTile(0);
            }
            else 
            {
                SpawnTile(Random.Range(0,tilePrefabs.Length));
            }
        }
        
    }

    // Update is called once per frame
    void Update()
    {
        if (playerTransform.position.z - 35 > zSpawn - (numberofTiles * tileLength))
        {
            SpawnTile(Random.Range(0,tilePrefabs.Length));
            DeleteTile();
        }
        
    }
    public void SpawnTile(int tileIndex)
    {
        GameObject go = Instantiate(tilePrefabs[tileIndex], transform.forward * zSpawn, transform.rotation);
        activeTiles.Add(go);
        zSpawn += tileLength;
    }

    private void DeleteTile()
    {
        Destroy(activeTiles[0]);
        activeTiles.RemoveAt(0);
    }
}
